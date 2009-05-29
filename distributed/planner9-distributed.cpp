#include "planner9-distributed.hpp"
#include "../core/relations.hpp"
#include "../core/planner9.hpp"
#include "../core/tasks.hpp"
#include <boost/cast.hpp>
#include <QTcpSocket>
#include <stdexcept>

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
	connect(&tcpServer, SIGNAL(dataReady()), SLOT(networkDataAvailable()));
	
	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}
}

void SlavePlanner9::newConnection() {
	
	tcpServer.close();
	QTcpSocket *clientConnection = tcpServer.nextPendingConnection();
	stream.setDevice(clientConnection);
	
	connect(clientConnection, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(clientConnection, SIGNAL(readyRead()), SLOT(networkDataAvailable()));
	
}

void SlavePlanner9::disconnected() {
	if (planner) {
		killTimer(timerId);
		delete planner;
		planner = 0;
	}
	
	clientConnection->deleteLater();
	
	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}
}


void SlavePlanner9::networkDataAvailable() {
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
			} else {
				// no more nodes, report failure
				stream.write(CMD_NOPLAN_FOUND);
				killPlanner();
			}
		} else {
			// report plan
			for (SimplePlanner9::Plans::const_iterator it = planner->plans.begin(); it != planner->plans.end(); ++it) {
				stream.write(CMD_PLAN_FOUND);
				stream.write(planner->plans.front());
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
	connect(client, SIGNAL(readyRead()), SLOT(networkDataAvailable()));
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
		QTcpSocket* client(it.key());
		it.value() = Planner9::InfiniteCost;
		sendScope(client);
	}
	
	// if one client
	if (!clients.empty()) {
		sendInitialNode(clients.begin().key());
	}
}

void MasterPlanner9::planFound(const Plan& plan) {
	std::cout << "plan:\n" << plan << std::endl;
}

void MasterPlanner9::noPlanFound() {
	std::cout << "no plan." << std::endl;
}

void MasterPlanner9::clientConnected() {
	QTcpSocket* client(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	clients[client] = Planner9::InfiniteCost;
	
	sendScope(client);
	
	if (clients.size() == 1) {
		sendInitialNode(clients.begin().key());
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

void MasterPlanner9::networkDataAvailable() {
	QTcpSocket* client(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	stream.setDevice(client);
	
	// fetch command from client
	Command cmd(stream.read<Command>());
	
	switch (cmd) {
		// cost 
		case CMD_CURRENT_COST: {
			Planner9::Cost cost(stream.read<Planner9::Cost>());
			clients[client] = cost;
			
			// only one client, return
			if (clients.size() <= 1)
				return;
			
			// read the clients map to see whether another one has lower cost
			for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
				if (it.value() < cost)
					return; // another has lower cost, do nothing
			}
			
			// we have lowest cost, get our best node
			stream.write<Command>(CMD_GET_NODE);
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
				if (it.value() > highestCost) {
					highestCost = it.value();
					highestCostIt = it;
				}
			}
			
			// send the node to it
			sendNode(highestCostIt.key(), node);
		} break;
		
		// plan
		case CMD_PLAN_FOUND: {
			const Plan plan(stream.read<Plan>());
			
			// tell all clients to stop searching
			for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
				QTcpSocket* client(it.key());
				sendStop(client);
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

void MasterPlanner9::sendScope(QTcpSocket* client) {
	stream.setDevice(client);
	stream.write(CMD_PROBLEM_SCOPE);
	stream.write(problem.scope);
}

void MasterPlanner9::sendInitialNode(QTcpSocket* client) {
	stream.setDevice(client);
	stream.write(CMD_PUSH_NODE);
	stream.write(*initialNode);
}

void MasterPlanner9::sendNode(QTcpSocket* client, const SimplePlanner9::SearchNode& node) {
	stream.setDevice(client);
	stream.write(CMD_PUSH_NODE);
	stream.write(node);
}

void MasterPlanner9::sendStop(QTcpSocket* client) {
	stream.setDevice(client);
	stream.write(CMD_STOP);
}
	
