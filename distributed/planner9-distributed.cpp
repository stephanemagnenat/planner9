#include "planner9-distributed.hpp"
#include "../core/planner9.hpp"
#include "../core/tasks.hpp"
#include "../core/relations.hpp"
#include <QTcpSocket>
#include <stdexcept>

// force the use of spetialized versions

template<> void Serializer::write(const Command& cmd);
template<> void Serializer::write(const Scope& scope);
template<> void Serializer::write(const Task& task);
template<> void Serializer::write(const Plan& plan);
template<> void Serializer::write(const Atom& atom);
template<> void Serializer::write(const CNF& cnf);
template<> void Serializer::write(const State& state);
template<> void Serializer::write(const TaskNetwork& network);
//template<> void Serializer::write(const Planner9::SearchNode*& node);

template<> Command Serializer::read();
template<> Scope Serializer::read();
template<> Task Serializer::read();
template<> Plan Serializer::read();
template<> Atom Serializer::read();
template<> CNF Serializer::read();
template<> State Serializer::read();
template<> TaskNetwork Serializer::read();
template<> Planner9::SearchNode* Serializer::read();


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
		SimplePlanner9::SearchNode* node(stream.read<SimplePlanner9::SearchNode*>());
		if (planner)
			planner->pushNode(node);
		else
			delete node;
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
		if (planner) {
			delete planner;
		} else {
			timerId = startTimer(0);
		}
		planner = new SimplePlanner9(scope);
	} break;
		
	default:
		throw std::runtime_error(tr("Unknown command received: %0").arg(cmd).toStdString());
		break;
	}
}

void SlavePlanner9::timerEvent(QTimerEvent *event) {
	if (planner && (clientConnection->state() == QAbstractSocket::ConnectedState)) {
		// plan for a while
		planner->plan(100);
		
		// if plan found
		if (planner->plans.empty()) {
			// report progress to master
			if (!planner->nodes.empty()) {
				stream.write(CMD_CURRENT_COST);
				stream.write(planner->nodes.begin()->first);
			}
		} else {
			// report plan
			for (SimplePlanner9::Plans::const_iterator it = planner->plans.begin(); it != planner->plans.end(); ++it) {
				stream.write(CMD_PLAN);
				stream.write(planner->plans.front());
			}
			killTimer(timerId);
			delete planner;
			planner = 0;
		}
	}
}
