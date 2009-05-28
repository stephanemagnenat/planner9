#include "planner9-distributed.hpp"
#include "../core/tasks.hpp"
#include <QTcpSocket>
#include <stdexcept>

Serializer::Serializer(const Domain& domain) :
	domain(domain) {
}

template<>
void Serializer::write(const Command& cmd) {
	write<quint16>(cmd);
}

template<>
void Serializer::write(const Scope& scope) {
	write<quint16>(scope.getSize());
	for (size_t i = 0; i < scope.getSize(); ++i)
		write(QString::fromStdString(scope.names[i]));
}

template<>
void Serializer::write(const Task& task) {
	write<quint16>(domain.getHeadIndex(task.head));
	for (Scope::Indices::const_iterator it = task.params.begin(); it != task.params.end(); ++it)
		write<quint16>(*it);
}	

template<>
void Serializer::write(const Plan& plan) {
	write<quint16>(plan.size());
	for (Plan::const_iterator it = plan.begin(); it != plan.end(); ++it)
		write(*it);
}

template<>
void Serializer::write(const Atom& atom) {
	write<quint16>(domain.getRelationIndex(atom.relation));
	for (Scope::Indices::const_iterator it = atom.params.begin(); it != atom.params.end(); ++it)
		write<quint16>(*it);
}

template<>
void Serializer::write(const CNF& cnf) {
	write<quint16>(cnf.size());
	for (CNF::const_iterator it = cnf.begin(); it != cnf.end(); ++it) {
		const Clause& clause(*it);
		write<quint16>(clause.size());
		for (Clause::const_iterator jt = clause.begin(); jt != clause.end(); ++jt) {
			const Literal& literal(*jt);
			write(literal.atom);
			write(literal.negated);
		}
	}
}

template<>
void Serializer::write(const State& state) {
	const size_t atomsCount(state.atoms.size());
	write<quint16>(atomsCount);
	for (State::AtomsPerRelation::const_iterator it = state.atoms.begin(); it != state.atoms.end(); ++it) {
		const State::AtomSet& atomSet(it->second);
		for (State::AtomSet::const_iterator jt = atomSet.begin(); jt != atomSet.end(); ++jt) {
			write(*jt);
		}
	}
}

template<>
void Serializer::write(const TaskNetwork& taskNetwork) {
/*
	const size_t tasksCount(taskNetwork.first.size() + taskNetwork.predecessors.size());
	*this << quint16(tasksCount);
	for (TaskNetwork::Tasks::const_iterator it = taskNetwork.first.begin(); it != taskNetwork.first.end(); ++it)
		*this << *(*it);
	for (TaskNetwork::Predecessors::const_iterator it = taskNetwork.predecessors.begin(); it != taskNetwork.predecessors.end(); ++it)
		*this << *(it->first);
	// TODO: fix this
		*/
}

void Serializer::write(const Planner9::SearchNode*& node) {
	write(node->plan);
	write(node->network);
	write(quint16(node->allocatedVariablesCount));
	write(quint16(node->cost));
	write(node->preconditions);
	write(node->state);
}

template<>
Command Serializer::read() {
	return Command(read<quint16>());
}

template<>
Scope Serializer::read() {
	Scope scope;
	quint16 size(read<quint16>());
	scope.names.resize(size);
	for (size_t i = 0; i < size; ++i) {
		const QString string(read<QString>());
		scope.names[i] = string.toStdString();
	}
	return scope;
}

template<>
Task Serializer::read()  {
	quint16 headIndex(read<quint16>());
	const Head* head(domain.getHead(headIndex));
	Scope::Indices indices;
	indices.reserve(head->getParamsCount());
	for (size_t i = 0; i < head->getParamsCount(); ++i) {
		const quint16 value(read<quint16>());
		indices.push_back(value);
	}
	return Task(head, indices);
}

template<>
Plan Serializer::read()  {
	// TODO
	return Plan();
}

template<>
Atom Serializer::read()  {
	// TODO
	return Atom(0, Scope::Indices());
}

template<>
CNF Serializer::read()  {
	// TODO
	return CNF();
}
	
template<>
State Serializer::read() {
	// TODO
	return State();
}
	
template<>
TaskNetwork Serializer::read()  {
	// TODO
	return TaskNetwork();
}

template<>
Planner9::SearchNode* Serializer::read()  {
	// TODO
	return 0;
}



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
