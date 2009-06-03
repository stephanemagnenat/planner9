#include "serializer.hpp"
#include "../core/relations.hpp"

Serializer::Serializer(const Domain& domain) :
	domain(domain) {
}

Serializer::Serializer(QIODevice * d, const Domain& domain) :
	QDataStream(d), 
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
	for (Variables::const_iterator it = task.params.begin(); it != task.params.end(); ++it)
		write<quint16>(it->index);
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
	for (Variables::const_iterator it = atom.params.begin(); it != atom.params.end(); ++it)
		write<quint16>(it->index);
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
	size_t atomsCount(0);
	
	for (State::AtomsPerRelation::const_iterator it = state.atoms.begin(); it != state.atoms.end(); ++it) {
		const State::AtomSet& atomSet(it->second);
		atomsCount += atomSet.size();
	}
	
	write<quint16>(atomsCount);
	for (State::AtomsPerRelation::const_iterator it = state.atoms.begin(); it != state.atoms.end(); ++it) {
		const State::AtomSet& atomSet(it->second);
		for (State::AtomSet::const_iterator jt = atomSet.begin(); jt != atomSet.end(); ++jt) {
			write(*jt);
		}
	}
}

template<>
void Serializer::write(const TaskNetwork& network) {
	typedef std::map<TaskNetwork::Node*, size_t> NodesMap;
	NodesMap nodesMap;
	size_t nodesCount = 0;
	
	// fill nodes map
	for(TaskNetwork::Tasks::const_iterator it = network.first.begin(); it != network.first.end(); ++it) {
		nodesMap[*it] = nodesCount++;
	}
	for (TaskNetwork::Predecessors::const_iterator it = network.predecessors.begin(); it != network.predecessors.end(); ++it) {
		nodesMap[it->first] = nodesCount++;
	}
	
	// store first
	write<quint16>(network.first.size());
	for(TaskNetwork::Tasks::const_iterator it = network.first.begin(); it != network.first.end(); ++it) {
		const TaskNetwork::Node* node(*it);
		write(node->task);
		write<quint16>(node->successors.size());
		for (TaskNetwork::Tasks::const_iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt) {
			write<quint16>(nodesMap[*jt]);
		}
	}
	// store predecessors
	write<quint16>(network.predecessors.size());
	for (TaskNetwork::Predecessors::const_iterator it = network.predecessors.begin(); it != network.predecessors.end(); ++it) {
		const TaskNetwork::Node* node(it->first);
		write(node->task);
		write<quint16>(node->successors.size());
		for (TaskNetwork::Tasks::const_iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt) {
			write<quint16>(nodesMap[*jt]);
		}
		write<quint16>(it->second);
	}
}

template<>
void Serializer::write(const Planner9::SearchNode& node) {
	write(node.plan);
	write(node.network);
	write(quint16(node.allocatedVariablesCount));
	write(quint16(node.cost));
	write(node.preconditions);
	write(node.state);
}

template<>
Command Serializer::read() {
	return Command(read<quint16>());
}

template<>
Scope Serializer::read() {
	Scope scope;
	const quint16 size(read<quint16>());
	scope.names.resize(size);
	for (size_t i = 0; i < size; ++i) {
		const QString string(read<QString>());
		scope.names[i] = string.toStdString();
	}
	return scope;
}

template<>
Task Serializer::read() {
	const quint16 headIndex(read<quint16>());
	const Head* head(domain.getHead(headIndex));
	Variables variables;
	variables.reserve(head->getParamsCount());
	for (size_t i = 0; i < head->getParamsCount(); ++i) {
		const quint16 value(read<quint16>());
		variables.push_back(Variable(value));
	}
	return Task(head, variables);
}

template<>
Plan Serializer::read() {
	const size_t planSize(read<quint16>());
	Plan plan;
	plan.reserve(planSize);
	for (size_t i = 0; i < planSize; ++i)
		plan.push_back(read<Task>());
	return plan;
}

template<>
Atom Serializer::read() {
	const Relation* relation(domain.getRelation(read<quint16>()));
	Variables variables;
	variables.reserve(relation->arity);
	for (size_t i = 0; i < relation->arity; ++i)
		variables.push_back(Variable(read<quint16>()));
	return Atom(relation, variables);
}

template<>
CNF Serializer::read() {
	CNF cnf;
	const size_t cnfSize(read<quint16>());
	cnf.reserve(cnfSize);
	for (size_t i = 0; i < cnfSize; ++i) {
		Clause clause;
		const size_t clauseSize(read<quint16>());
		clause.reserve(clauseSize);
		for (size_t j = 0; j < clauseSize; ++j) {
			const Atom atom(read<Atom>());
			const bool negated(read<bool>());
			clause.push_back(Literal(atom, negated));
		}
		cnf.push_back(clause);
	}
	return cnf;
}
	
template<>
State Serializer::read() {
	State state;
	size_t atomsCount(read<quint16>());
	for (size_t i = 0; i < atomsCount; ++i) {
		state.insert(read<Atom>());
	}
	return state;
}
	
template<>
TaskNetwork Serializer::read() {
	TaskNetwork::Tasks nodes;
	
	// Note: for efficiency reasons, we temporary store indexes in Node*,
	// which is unclean but faster than building an ad-hoc map.
	TaskNetwork network;
	
	// read first nodes
	const size_t firstSize(read<quint16>());
	network.first.reserve(firstSize);
	for (size_t i = 0; i < firstSize; ++i) {
		const Task task(read<Task>());
		TaskNetwork::Node* node(new TaskNetwork::Node(task));
		const size_t successorsCount(read<quint16>());
		node->successors.resize(successorsCount);
		for (size_t j = 0; j < successorsCount; ++j) {
			node->successors.push_back((TaskNetwork::Node*)read<quint16>());
		}
		nodes.push_back(node);
		network.first.push_back(node);
	}
	
	// read predecessors nodes
	const size_t predecessorsSize(read<quint16>());
	for (size_t i = 0; i < predecessorsSize; ++i) {
		const Task task(read<Task>());
		TaskNetwork::Node* node(new TaskNetwork::Node(task));
		const size_t successorsCount(read<quint16>());
		node->successors.resize(successorsCount);
		for (size_t j = 0; j < successorsCount; ++j) {
			node->successors.push_back((TaskNetwork::Node*)read<quint16>());
		}
		nodes.push_back(node);
		network.predecessors[node] = read<quint16>();
	}
	
	// resolve cross-references
	for(TaskNetwork::Tasks::iterator it = network.first.begin(); it != network.first.end(); ++it) {
		TaskNetwork::Node* node(*it);
		for (TaskNetwork::Tasks::iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt) {
			*jt = nodes[(size_t)*jt];
		}
	}
	for (TaskNetwork::Predecessors::iterator it = network.predecessors.begin(); it != network.predecessors.end(); ++it) {
		TaskNetwork::Node* node(it->first);
		for (TaskNetwork::Tasks::iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt) {
			*jt = nodes[(size_t)*jt];
		}
	}
	
	return network;
}

template<>
Planner9::SearchNode Serializer::read() {
	const Plan plan(read<Plan>());
	const TaskNetwork network(read<TaskNetwork>());
	const size_t allocatedVariablesCount(read<quint16>());
	const Cost cost(read<quint16>());
	const CNF preconditions(read<CNF>());
	const State state(read<State>());
	return Planner9::SearchNode(plan, network, allocatedVariablesCount, cost, preconditions, state);				
}

///

/*
TODO: examples for safe network operations
bool sendCurrentCost(const Domain& domain, QAbstractSocket* socket, const Planner9::Cost& cost) {
	QBuffer buffer;
	Serializer s(&buffer, domain);
	s.write(CMD_CURRENT_COST);
	s.write(planner->nodes.begin()->first);
	
	QDataStream stream(socket);
	stream << quint32(buffer.size());
	stream.writeRawData(buffer.data(), buffer.size());
	
	return stream.status() == QDataStream::Ok;
}
*/
