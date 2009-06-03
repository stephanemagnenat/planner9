#include "planner9-distributed.h"
#include "../core/relations.hpp"
#include "../core/planner9.hpp"
#include "../core/tasks.hpp"
#include <boost/cast.hpp>
#include <QTcpSocket>
#include <stdexcept>

#include "planner9-distributed.moc"


// force the use of specialized versions

template<> void Serializer::write(const Command& cmd);
template<> void Serializer::write(const Scope& scope);
template<> void Serializer::write(const Task& task);
template<> void Serializer::write(const Plan& plan);
template<> void Serializer::write(const Atom& atom);
template<> void Serializer::write(const CNF& cnf);
template<> void Serializer::write(const State& state);
template<> void Serializer::write(const TaskNetwork& network);
template<> void Serializer::write(const Planner9::SearchNode& node);

template<> Command Serializer::read();
template<> Scope Serializer::read();
template<> Task Serializer::read();
template<> Plan Serializer::read();
template<> Atom Serializer::read();
template<> CNF Serializer::read();
template<> State Serializer::read();
template<> TaskNetwork Serializer::read();
template<> Planner9::SearchNode Serializer::read();


SlavePlanner9::SlavePlanner9(const Domain& domain, std::ostream* debugStream):
	planner(0),
	device(0),
	stream(domain),
	debugStream(debugStream) {

	tcpServer.setMaxPendingConnections(1);

	connect(&tcpServer, SIGNAL(newConnection()), SLOT(newConnection()));

	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}

	std::cout << "Listening on port " << tcpServer.serverPort() << std::endl;
}

void SlavePlanner9::newConnection() {
	Q_ASSERT(device == 0);

	QTcpSocket* socket = tcpServer.nextPendingConnection();
	Q_ASSERT(socket);
	tcpServer.close();

	device = new ChunkedDevice(socket);
	stream.setDevice(device);

	connect(device, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(device, SIGNAL(readyRead()), SLOT(messageAvailable()));

	if (debugStream) *debugStream << "Connection from " << socket->peerAddress().toString().toStdString() << std::endl;
}

void SlavePlanner9::disconnected() {
	if (planner) {
		killTimer(timerId);
		delete planner;
		planner = 0;
	}

	if (debugStream) *debugStream << "Connection closed"  << std::endl;

	device->deleteLater();
	device = 0;

	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}

	std::cout << "Listening on port " << tcpServer.serverPort() << std::endl;
}


void SlavePlanner9::messageAvailable() {
	// fetch command from master
	Command cmd(stream.read<Command>());

	qDebug() << "Cmd" << cmd;

	switch (cmd) {
		// new node to insert
		case CMD_PUSH_NODE: {
			SimplePlanner9::SearchNode* node(new SimplePlanner9::SearchNode(stream.read<SimplePlanner9::SearchNode>()));
			//std::cerr << "received:\n" << (*node) << "\ndone" << std::endl;
			assert(planner);
			if (planner->nodes.empty())
				timerId = startTimer(0);
			planner->pushNode(node);
		} break;

		// node to send
		case CMD_GET_NODE: {
			if (planner && !planner->nodes.empty()) {
				stream.write(CMD_PUSH_NODE);
				stream.write(*(planner->nodes.begin()->second));
				if (debugStream) *debugStream  << "sending:\n" << *(planner->nodes.begin()->second) << "\ndone" << std::endl;
				device->flush();
				planner->nodes.erase(planner->nodes.begin());
			}
		} break;

		// new problem scope
		case CMD_PROBLEM_SCOPE: {
			if (planner)
				delete planner;
			Scope scope(stream.read<Scope>());
			planner = new SimplePlanner9(scope, debugStream);
		} break;

		// stop processing
		case CMD_STOP: {
			if (planner) {
				killTimer(timerId);
				delete planner;
				planner = 0;
			}
		} break;

		default:
			throw std::runtime_error(tr("Unknown command received: %0").arg(cmd).toStdString());
			break;
	}
}

void SlavePlanner9::timerEvent(QTimerEvent *event) {
	if (planner) {
		// plan for a while
		planner->plan(100);

		// if plan found
		if (planner->plans.empty()) {
			// report progress to master
			if (!planner->nodes.empty()) {
				qDebug() << "current cost" << planner->nodes.begin()->first;
				stream.write(CMD_CURRENT_COST);
				stream.write(planner->nodes.begin()->first);
				device->flush();
			} else {
				// no more nodes, report failure
				qDebug() << "no plan found";
				stream.write(CMD_NOPLAN_FOUND);
				device->flush();
				killPlanner();
			}
		} else {
			// report plan
			qDebug() << "plan found";
			for (SimplePlanner9::Plans::const_iterator it = planner->plans.begin(); it != planner->plans.end(); ++it) {
				stream.write(CMD_PLAN_FOUND);
				stream.write(planner->plans.front());
				device->flush();
			}
			killPlanner();
		}
	}
}

void SlavePlanner9::killPlanner() {
	killTimer(timerId);
	delete planner;
	planner = 0;
}


MasterPlanner9::Client::Client() :
	device(0),
	cost(Planner9::InfiniteCost) {
}

MasterPlanner9::Client::Client(ChunkedDevice* device) :
	device(device),
	cost(Planner9::InfiniteCost) {
}

MasterPlanner9::MasterPlanner9(const Domain& domain, std::ostream* debugStream):
	initialNode(0),
	stream(domain),
	debugStream(debugStream) {
}

MasterPlanner9::~MasterPlanner9() {
	for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		QTcpSocket* client(it.key());
		client->disconnectFromHost();
	}
	if (initialNode)
		delete initialNode;
}

bool MasterPlanner9::connectToSlave(const QString& hostName, quint16 port) {
	QTcpSocket* client(new QTcpSocket);
	client->connectToHost(hostName, port);
	connect(client, SIGNAL(connected()), SLOT(clientConnected()));
	connect(client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
	connect(client, SIGNAL(disconnected()), client, SLOT(deleteLater()));
	connect(client, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(clientConnectionError(QAbstractSocket::SocketError)));
	connect(client, SIGNAL(error(QAbstractSocket::SocketError)), client, SLOT(deleteLater()));
}

void MasterPlanner9::plan(const Problem& problem) {
	if (debugStream) {
		*debugStream << Scope::setScope(problem.scope);
		*debugStream << "initial state: "<< problem.state << std::endl;
		*debugStream << "initial network: " << problem.network << std::endl;
	}
	this->problem = problem;

	if (initialNode)
		delete initialNode;
	initialNode = new Planner9::SearchNode(Plan(), problem.network, problem.scope.getSize(), 0, CNF(), problem.state);

	// send scope to every client to clear their current computations
	// TODO: handle possible race conditions
	// - locking until clearing ?
	for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
		Client& client = it.value();
		client.cost = Planner9::InfiniteCost;
		sendScope(client.device);
	}

	// if one client
	if (!clients.empty()) {
		sendInitialNode(clients.begin().value().device);
	}
}

void MasterPlanner9::planFound(const Plan& plan) {
	if (debugStream) *debugStream << "plan:\n" << plan << std::endl;
}

void MasterPlanner9::noPlanFound() {
	if (debugStream) *debugStream << "no plan." << std::endl;
}

void MasterPlanner9::clientConnected() {
	QTcpSocket* socket(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	ChunkedDevice* device = new ChunkedDevice(socket);
	connect(device, SIGNAL(readyRead()), SLOT(messageAvailable()));

	clients[socket] = Client(device);

	if (debugStream) *debugStream << "New client" << device;

	sendScope(device);

	if (clients.size() == 1) {
		if (debugStream) *debugStream << "Sending:\n" << *initialNode << "\ndone" << std::endl;
		sendInitialNode(device);
	}
}

void MasterPlanner9::clientDisconnected() {
	QTcpSocket* client(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	clients.remove(client);

	// TODO: manage disconnection, resend node of this one
}

void MasterPlanner9::clientConnectionError(QAbstractSocket::SocketError socketError) {
	QTcpSocket* client(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	clients.remove(client);

	// TODO: manage disconnection, resend node of this one
}

void MasterPlanner9::messageAvailable() {
	ChunkedDevice* device(boost::polymorphic_downcast<ChunkedDevice*>(sender()));
	QTcpSocket* socket = boost::polymorphic_downcast<QTcpSocket*>(device->parentDevice());
	stream.setDevice(device);

	// fetch command from client
	Command cmd(stream.read<Command>());
	qDebug() << "Cmd" << cmd;

	switch (cmd) {
		// cost
		case CMD_CURRENT_COST: {
			Planner9::Cost cost(stream.read<Planner9::Cost>());
			Client& client(clients[socket]);
			client.cost = cost;
			qDebug() << "Cost" << client.device << cost;

			// only one client, return
			if (clients.size() <= 1)
				return;

			// read the clients map to see whether another one has lower cost
			for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
				if (it.value().cost < cost)
					return; // another has lower cost, do nothing
			}

			qDebug() << "Min cost" << client.device;

			// we have lowest cost, get our best node
			stream.write<Command>(CMD_GET_NODE);
			device->flush();
		} break;

		// node
		case CMD_PUSH_NODE: {
			SimplePlanner9::SearchNode node(stream.read<SimplePlanner9::SearchNode>());

			// only one client, return
			if (clients.size() <= 1)
				return;

			// find the client with the highest score
			ClientsMap::iterator highestCostIt;
			Planner9::Cost highestCost(0);
			for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
				Planner9::Cost cost = it.value().cost;
				if (cost > highestCost) {
					highestCost = cost;
					highestCostIt = it;
				}
			}

			qDebug() << "Load balance" << device << highestCostIt.value().device;

			// send the node to it
			if (debugStream) *debugStream  << "sending:\n" << node << "\ndone" << std::endl;
			sendNode(highestCostIt.value().device, node);
		} break;

		// plan
		case CMD_PLAN_FOUND: {
			const Plan plan(stream.read<Plan>());

			// tell all clients to stop searching
			for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
				sendStop(it.value().device);
			}

			// print the plan
			planFound(plan);
		} break;

		// no plan for this client
		case CMD_NOPLAN_FOUND: {
			// TODO: check whether a client still has work, otherwise report general failure
		} break;

		default:
			throw std::runtime_error(tr("Unknown command received: %0").arg(cmd).toStdString());
			break;
	}
}

void MasterPlanner9::sendScope(ChunkedDevice* device) {
	stream.setDevice(device);
	stream.write(CMD_PROBLEM_SCOPE);
	stream.write(problem.scope);
	device->flush();
}

void MasterPlanner9::sendInitialNode(ChunkedDevice* device) {
	stream.setDevice(device);
	stream.write(CMD_PUSH_NODE);
	stream.write(*initialNode);
	device->flush();
}

void MasterPlanner9::sendNode(ChunkedDevice* device, const SimplePlanner9::SearchNode& node) {
	stream.setDevice(device);
	stream.write(CMD_PUSH_NODE);
	stream.write(node);
	device->flush();
}

void MasterPlanner9::sendStop(ChunkedDevice* device) {
	stream.setDevice(device);
	stream.write(CMD_STOP);
	device->flush();
}

