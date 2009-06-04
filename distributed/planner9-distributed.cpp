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


SlavePlanner9::SlavePlanner9(const Domain& domain, std::ostream* debugStream):
	timerId(-1),
	planner(0),
	chunkedDevice(0),
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
	Q_ASSERT(chunkedDevice == 0);
	
	QTcpSocket *clientConnection = tcpServer.nextPendingConnection();
	Q_ASSERT(clientConnection);
	tcpServer.close();
	
	chunkedDevice = new ChunkedDevice(clientConnection);
	stream.setDevice(chunkedDevice);
	
	connect(chunkedDevice, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(chunkedDevice, SIGNAL(readyRead()), SLOT(messageAvailable()));
	
	if (debugStream) *debugStream << "Connection from " << clientConnection->peerAddress().toString().toStdString() << std::endl;
}

void SlavePlanner9::disconnected() {
	stopTimer();
	killPlanner();
	
	if (debugStream) *debugStream << "Connection closed"  << std::endl;
	
	chunkedDevice->deleteLater();
	chunkedDevice = 0;
	
	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}
	
	std::cout << "Listening on port " << tcpServer.serverPort() << std::endl;
}


void SlavePlanner9::messageAvailable() {
	while (chunkedDevice->isMessage()) {
		// fetch command from master
		Command cmd(stream.read<Command>());
		
		qDebug() << "Cmd" << cmd;
		
		switch (cmd) {
			// new node to insert
			case CMD_PUSH_NODE: {
				SimplePlanner9::SearchNode* node(new SimplePlanner9::SearchNode(stream.read<SimplePlanner9::SearchNode>()));
				//std::cerr << "received:\n" << (*node) << "\ndone" << std::endl;
				qDebug() << "inserting node of cost" << node->getTotalCost();
				assert(planner);
				planner->pushNode(node);
				runTimer();
			} break;
				
			// node to send
			case CMD_GET_NODE: {
				if (planner && !planner->nodes.empty()) {
					stream.write(CMD_PUSH_NODE);
					stream.write(*(planner->nodes.begin()->second));
					if (debugStream) *debugStream  << "sending:\n" << *(planner->nodes.begin()->second) << "\ndone" << std::endl;
					chunkedDevice->flush();
					qDebug() << "sending node of cost" << planner->nodes.begin()->first;
					planner->nodes.erase(planner->nodes.begin());
				}
			} break;
				
			// new problem scope
			case CMD_PROBLEM_SCOPE: {
				killPlanner();
				const Scope scope(stream.read<Scope>());
				runPlanner(scope);
			} break;
				
			// stop processing
			case CMD_STOP: {
				stopTimer();
				killPlanner();
				// acknowledge stop
				stream.write(CMD_STOP);
				chunkedDevice->flush();
			} break;
				
			default:
				throw std::runtime_error(tr("Unknown command received: %0").arg(cmd).toStdString());
				break;
		}
	}	
}

void SlavePlanner9::timerEvent(QTimerEvent *event) {
	if (planner) {
		// TODO: improve the performances of this loop by using a thread
		// and not polling of the event loop
		// plan for a specified duration
		QTime sliceStartingTime(QTime::currentTime());
		while (sliceStartingTime.msecsTo(QTime::currentTime()) < 10) {
			if (!planner->plan(1))
				break;
		}
		
		planner->plan(1);
		
		// if plan found
		if (planner->plans.empty()) {
			// report progress to master
			if (!planner->nodes.empty()) {
				const Planner9::Cost currentCost(planner->nodes.begin()->first);
				if (currentCost != lastSentCost) {
					qDebug() << "new current cost" << currentCost;
					stream.write(CMD_CURRENT_COST);
					stream.write(planner->nodes.begin()->first);
					chunkedDevice->flush();
					lastSentCost = currentCost;
				}
			} else {
				// no more nodes, report failure
				qDebug() << "no plan found";
				stream.write(CMD_NOPLAN_FOUND);
				chunkedDevice->flush();
				stopTimer();
			}
		} else {
			// report plan
			qDebug() << "plan found";
			for (SimplePlanner9::Plans::const_iterator it = planner->plans.begin(); it != planner->plans.end(); ++it) {
				stream.write(CMD_PLAN_FOUND);
				stream.write(planner->plans.front());
				chunkedDevice->flush();
				// TODO: if required, put into idle until new problem
			}
			stopTimer();
		}
	}
}

void SlavePlanner9::runPlanner(const Scope& scope) {
	planner = new SimplePlanner9(scope, debugStream);
	lastSentCost = Planner9::InfiniteCost;
}

void SlavePlanner9::killPlanner() {
	if (planner) {
		delete planner;
		planner = 0;
	}
}

void SlavePlanner9::runTimer() {
	timerId = startTimer(0);
}

void SlavePlanner9::stopTimer() {
	if (timerId != -1) {
		killTimer(timerId);
		timerId = -1;
	}
}

/////

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
	debugStream(debugStream),
	stoppingCount(0),
	newSearch(false) {
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
	newSearch = true;
	
	stopClients();
	
	startSearchIfReady();
	
	planStartTime = QTime::currentTime();
}

void MasterPlanner9::planFound(const Plan& plan) {
	const int planningDuration(planStartTime.msecsTo(QTime::currentTime()));
	if (debugStream) *debugStream << "After " << planningDuration << " ms, plan:\n" << plan << std::endl;
}

void MasterPlanner9::noPlanFound() {
	const int planningDuration(planStartTime.msecsTo(QTime::currentTime()));
	if (debugStream) *debugStream << "After " << planningDuration << " ms, no plan." << std::endl;
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
	Client& client(clients[socket]);
	
	while (device->isMessage()) {
		processMessage(client);
	}
}

void MasterPlanner9::processMessage(Client& client) {
	// fetch command from client
	//qDebug() << "Cmd pre";
	stream.setDevice(client.device);
	Command cmd(stream.read<Command>());
	qDebug() << "Cmd" << cmd;
	
	switch (cmd) {
		// cost 
		case CMD_CURRENT_COST: {
			Planner9::Cost cost(stream.read<Planner9::Cost>());
			
			// ignore if stopping
			if (stoppingCount > 0)
				return;
			
			client.cost = cost;
			
			std::cerr << "Cost map: ";
			for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
				std::cerr << it.value().device << ": " << it.value().cost << "\t";
			}
			std::cerr << std::endl;
			
			// only one client, return
			if (clients.size() <= 1)
				return;
			
			// read the clients map to see whether another one has lower cost
			for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
				if (it.value().cost < cost)
					return; // another has lower cost, do nothing
			}
			
			std::cerr  << "Min cost " << client.device << std::endl;
			
			// we have lowest cost, get our best node
			sendGetNode(client.device);
		} break;
		
		// node
		case CMD_PUSH_NODE: {
			SimplePlanner9::SearchNode node(stream.read<SimplePlanner9::SearchNode>());
			
			// ignore if stopping
			if (stoppingCount > 0)
				return;
			
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
			
			std::cerr  << "Load balancing " << client.device << " to " <<highestCostIt.value().device << " cost " << node.getTotalCost() << std::endl;
			
			// send the node to it and update cost
			if (debugStream) *debugStream  << "sending:\n" << node << "\ndone" << std::endl;
			sendNode(highestCostIt.value().device, node);
			highestCostIt.value().cost = std::min(node.getTotalCost(), highestCostIt.value().cost);
		} break;
		
		// plan
		case CMD_PLAN_FOUND: {
			const Plan plan(stream.read<Plan>());
			
			// ignore if stopping
			if (stoppingCount > 0)
				return;
			
			stopClients();
			
			// print the plan
			planFound(plan);
		} break;
		
		// no plan for this client
		case CMD_NOPLAN_FOUND: {
			
			// ignore if stopping
			if (stoppingCount > 0)
				return;
			
			client.cost = Planner9::InfiniteCost;
			
			bool stillSearching(false);
			for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
				if (it.value().cost != Planner9::InfiniteCost) {
					stillSearching = true;
				}
			}
			
			if (stillSearching == false) {
				// notify failure
				noPlanFound();
			}
		} break;
			
		// stop acknowledge
		case CMD_STOP: {
			stoppingCount--;
			startSearchIfReady();
		} break;
			
		default:
			throw std::runtime_error(tr("Unknown command received: %0").arg(cmd).toStdString());
			break;
	}
}

void MasterPlanner9::startSearchIfReady() {
	// ignore if stopping
	if (stoppingCount > 0)
		return;
	
	// do not redo the same search twice
	if (!newSearch)
		return;
	newSearch = false;
	
	// send scope to every client to clear their current computations
	for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
		Client& client = it.value();
		client.cost = Planner9::InfiniteCost;
		sendScope(client.device);
	}
	
	// if one client is present
	if (!clients.empty()) {
		sendInitialNode(clients.begin().value().device);
	}
}

void MasterPlanner9::stopClients() {
	// tell all clients to stop searching
	for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		sendStop(it.value().device);
	}
	
	// wait until all clients have acknowledged the stop
	stoppingCount = clients.size();
}

void MasterPlanner9::sendGetNode(ChunkedDevice* device) {
	stream.setDevice(device);
	stream.write(CMD_GET_NODE);
	device->flush();
}

void MasterPlanner9::sendScope(ChunkedDevice* device) {
	stream.setDevice(device);
	stream.write(CMD_PROBLEM_SCOPE);
	stream.write(problem.scope);
	device->flush();
}

void MasterPlanner9::sendInitialNode(ChunkedDevice* device) {
	sendNode(device, *initialNode);
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
	
