#include "planner9-distributed.h"
#include "../core/relations.hpp"
#include "../core/planner9.hpp"
#include "../core/tasks.hpp"
#include <boost/cast.hpp>
#include <QTcpSocket>
#include <stdexcept>

#include <planner9-distributed.moc>

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


SlavePlanner9::SlavePlanner9(const Domain& domain):
	planner(0),
	stream(domain) {
	
	tcpServer.setMaxPendingConnections(1);
	
	connect(&tcpServer, SIGNAL(newConnection()), SLOT(newConnection()));
	
	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}
	
	qDebug() << "listening on " << tcpServer.serverPort();
}

void SlavePlanner9::newConnection() {
	
	tcpServer.close();
	QTcpSocket *clientConnection = tcpServer.nextPendingConnection();
	this->chunkedDevice = new ChunkedDevice(clientConnection);
	stream.setDevice(chunkedDevice);
	
	connect(chunkedDevice, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(chunkedDevice, SIGNAL(readyRead()), SLOT(messageAvailable()));
	
}

void SlavePlanner9::disconnected() {
	if (planner) {
		killTimer(timerId);
		delete planner;
		planner = 0;
	}
	
	chunkedDevice->deleteLater();
	
	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}
}


void SlavePlanner9::messageAvailable() {
	// fetch command from master
	Command cmd(stream.read<Command>());
	
	switch (cmd) {
		// new node to insert
		case CMD_PUSH_NODE: {
			SimplePlanner9::SearchNode* node(new SimplePlanner9::SearchNode(stream.read<SimplePlanner9::SearchNode>()));
			assert(planner);
			if (planner->nodes.empty())
				timerId = startTimer(0);
			planner->pushNode(node);
		} break;
			
		// node to send
		case CMD_GET_NODE: {
			if (planner && !planner->nodes.empty()) {
				stream.write(CMD_PUSH_NODE);
				stream.write(planner->nodes.begin()->second);
				chunkedDevice->flush();
			}
		} break;
			
		// new problem scope
		case CMD_PROBLEM_SCOPE: {
			Scope scope(stream.read<Scope>());
			if (planner)
				delete planner;
			planner = new SimplePlanner9(scope);
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
				stream.write(CMD_CURRENT_COST);
				stream.write(planner->nodes.begin()->first);
				chunkedDevice->flush();
			} else {
				// no more nodes, report failure
				stream.write(CMD_NOPLAN_FOUND);
				chunkedDevice->flush();
				killPlanner();
			}
		} else {
			// report plan
			for (SimplePlanner9::Plans::const_iterator it = planner->plans.begin(); it != planner->plans.end(); ++it) {
				stream.write(CMD_PLAN_FOUND);
				stream.write(planner->plans.front());
				chunkedDevice->flush();
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

MasterPlanner9::MasterPlanner9(const Domain& domain):
	initialNode(0),
	stream(domain) {
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
	std::cout << Scope::setScope(problem.scope);
	std::cout << "initial state: "<< problem.state << std::endl;
	std::cout << "initial network: " << problem.network << std::endl;
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
	std::cout << "plan:\n" << plan << std::endl;
}

void MasterPlanner9::noPlanFound() {
	std::cout << "no plan." << std::endl;
}

void MasterPlanner9::clientConnected() {
	QTcpSocket* socket(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	ChunkedDevice* device = new ChunkedDevice(socket);
	connect(device, SIGNAL(readyRead()), SLOT(messageAvailable()));
	
	Client client;
	client.cost = Planner9::InfiniteCost;
	client.device = device;
	clients[socket] = client;
	
	sendScope(device);
	
	if (clients.size() == 1) {
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
	
	switch (cmd) {
		// cost 
		case CMD_CURRENT_COST: {
			Planner9::Cost cost(stream.read<Planner9::Cost>());
			clients[socket].cost = cost;
			
			// only one client, return
			if (clients.size() <= 1)
				return;
			
			// read the clients map to see whether another one has lower cost
			for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
				if (it.value().cost < cost)
					return; // another has lower cost, do nothing
			}
			
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
			
			// send the node to it
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
		};
			
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
	
