#include "tasks.hpp"
#include "domain.hpp"
#include <iostream>
#include <set>


Task::Task(const Head* head, const Variables& params):
	head(head),
	params(params) {
}

void Task::substitute(const Substitution& subst) {
	params.substitute(subst);
}

std::ostream& operator<<(std::ostream& os, const Task& task) {
	return os << task.head->name << "(" << task.params << ")";
}

Substitution Task::getSubstitution(const size_t taskScopeSize, const Variable::Index nextVariable) const {
	Substitution subst(params);
	assert (taskScopeSize >= params.size());
	size_t newVariables = taskScopeSize - params.size();
	for(size_t i = 0; i < newVariables; ++i) {
		subst.push_back(Variable(nextVariable + i));
	}
	return subst;
}

TaskNetwork::Node::Node(const Task& task) :
	task(task) {
}

TaskNetwork::TaskNetwork() {
	// does not do anything	
}

TaskNetwork::TaskNetwork(const TaskNetwork& that) {
	typedef std::map<const Node*,Node*> SuccessorsSubstitutionMap;
	SuccessorsSubstitutionMap successorsSubstitutionMap;
	
	// create a copy of that, build a map of substitution
	first.reserve(that.first.size());
	for (Tasks::const_iterator it = that.first.begin(); it != that.first.end(); ++it) {
		const Node* thatNode(*it);
		Node* thisNode(new Node(*thatNode));
		first.push_back(thisNode);
		successorsSubstitutionMap[thatNode] = thisNode;
	}
	for (Predecessors::const_iterator it = that.predecessors.begin(); it != that.predecessors.end(); ++it) {
		const Node* thatNode(it->first);
		Node* thisNode(new Node(*thatNode));
		predecessors[thisNode] = it->second;
		successorsSubstitutionMap[thatNode] = thisNode;
	}
	// fixup successors
	for (Tasks::const_iterator it = first.begin(); it != first.end(); ++it) {
		Node* node(*it);
		for (Tasks::iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt) {
			*jt = successorsSubstitutionMap[*jt];
		}
	}
	for (Predecessors::const_iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
		Node* node(it->first);
		for (Tasks::iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt) {
			*jt = successorsSubstitutionMap[*jt];
		}
	}
}

TaskNetwork& TaskNetwork::operator=(const TaskNetwork& that) {
	// first delete current content
	for (Tasks::iterator it = first.begin(); it != first.end(); ++it)
		 delete *it;
	for (Predecessors::iterator it = predecessors.begin(); it != predecessors.end(); ++it)
		delete it->first;
	first.clear();
	predecessors.clear();
	
	typedef std::map<const Node*,Node*> SuccessorsSubstitutionMap;
	SuccessorsSubstitutionMap successorsSubstitutionMap;
	
	// create a copy of that, build a map of substitution
	first.reserve(that.first.size());
	for (Tasks::const_iterator it = that.first.begin(); it != that.first.end(); ++it) {
		const Node* thatNode(*it);
		Node* thisNode(new Node(*thatNode));
		first.push_back(thisNode);
		successorsSubstitutionMap[thatNode] = thisNode;
	}
	for (Predecessors::const_iterator it = that.predecessors.begin(); it != that.predecessors.end(); ++it) {
		const Node* thatNode(it->first);
		Node* thisNode(new Node(*thatNode));
		predecessors[thisNode] = it->second;
		successorsSubstitutionMap[thatNode] = thisNode;
	}
	// fixup successors
	for (Tasks::const_iterator it = first.begin(); it != first.end(); ++it) {
		Node* node(*it);
		for (Tasks::iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt) {
			*jt = successorsSubstitutionMap[*jt];
		}
	}
	for (Predecessors::const_iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
		Node* node(it->first);
		for (Tasks::iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt) {
			*jt = successorsSubstitutionMap[*jt];
		}
	}
	
	return *this;
}

TaskNetwork::~TaskNetwork() {
	for (Tasks::iterator it = first.begin(); it != first.end(); ++it)
		 delete *it;
	for (Predecessors::iterator it = predecessors.begin(); it != predecessors.end(); ++it)
		delete it->first;
}

void TaskNetwork::substitute(const Substitution& subst) {
	for(Tasks::iterator it = first.begin(); it != first.end(); ++it) {
		(*it)->task.substitute(subst);
	}
	for(Predecessors::iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
		it->first->task.substitute(subst);
	}
}

TaskNetwork TaskNetwork::operator>>(const TaskNetwork& that) const {
	TaskNetwork thisCopy(*this);
	TaskNetwork thatCopy(that);
	
	size_t lasts = 0;
	for(Tasks::const_iterator it = thisCopy.first.begin(); it != thisCopy.first.end(); ++it) {
		Node* node(*it);
		if (node->successors.empty()) {
			++lasts;
			node->successors = thatCopy.first;
		}
	}
	for(Predecessors::const_iterator it = thisCopy.predecessors.begin(); it != thisCopy.predecessors.end(); ++it) {
		Node* node(it->first);
		if (node->successors.empty()) {
			++lasts;
			node->successors = thatCopy.first;
		}
	}
	
	for(Tasks::const_iterator it = thatCopy.first.begin(); it != thatCopy.first.end(); ++it) {
		Node* node(*it);
		thisCopy.predecessors[node] = lasts;
	}
	thisCopy.predecessors.insert(thatCopy.predecessors.begin(), thatCopy.predecessors.end());
	
	thatCopy.first.clear();
	thatCopy.predecessors.clear();

	return thisCopy;
}

void TaskNetwork::erase(size_t position) {
	Node* node = first[position];
	first.erase(first.begin() + position);
	
	for (Tasks::const_iterator it = node->successors.begin(); it != node->successors.end(); ++it) {
		Node* successor(*it);
		Predecessors::iterator preIt = predecessors.find(successor);
		assert(preIt != predecessors.end());
		if (preIt->second == 1) {
			predecessors.erase(preIt);
			first.push_back(successor);
		} else {
			preIt->second--;
		}
	}
	
	delete node;
}

void TaskNetwork::replace(size_t position, const TaskNetwork& that) {
	Node* replaced = first[position];
	
	TaskNetwork thatCopy(that);
	
	size_t lasts = 0;
	for(Tasks::const_iterator it = thatCopy.first.begin(); it != thatCopy.first.end(); ++it) {
		Node* node(*it);
		if (node->successors.empty()) {
			++lasts;
			node->successors = replaced->successors;
		}
	}
	for(Predecessors::const_iterator it = thatCopy.predecessors.begin(); it != thatCopy.predecessors.end(); ++it) {
		Node* node(it->first);
		if (node->successors.empty()) {
			++lasts;
			node->successors = replaced->successors;
		}
	}

	for(Tasks::iterator it = replaced->successors.begin(); it != replaced->successors.end(); ++it) {
		Node* node(*it);
		Predecessors::iterator preIt(predecessors.find(node));
		assert(preIt != predecessors.end());
		preIt->second += lasts - 1;
		if (preIt->second == 0) {
			// we have to move this node in the first
			first.push_back(node);
			predecessors.erase(preIt);
		}
	}
	
	delete replaced;
	first.erase(first.begin() + position);
	
	first.insert(first.end(), thatCopy.first.begin(), thatCopy.first.end());
	predecessors.insert(thatCopy.predecessors.begin(), thatCopy.predecessors.end());
	
	thatCopy.first.clear();
	thatCopy.predecessors.clear();
}

std::ostream& operator<<(std::ostream& os, const TaskNetwork& network) {
	typedef std::map<TaskNetwork::Node*, size_t> TasksIdsMap;
	TasksIdsMap tasksIdsMap;

	typedef std::set<TaskNetwork::Node*> TasksSet;
	TasksSet alreadySeen;
	TaskNetwork::Tasks workList = network.first;

	// print all tasks (nodes)
	size_t index = 0;
	while(index < workList.size()) {
		TaskNetwork::Node* node = workList[index];

		os << index << ":";
		tasksIdsMap[node] = index++;
		os << node->task;

		for(TaskNetwork::Tasks::const_iterator it = node->successors.begin(); it != node->successors.end(); ++it) {
			TaskNetwork::Node* succ = *it;
			if(alreadySeen.insert(succ).second) {
				workList.push_back(succ);
			}
		}

		if (index < workList.size())
			os << ", ";
	}

	if(workList.empty())
		os << "empty task network";
	else
		os << ": ";

	// print precedencies order constraints (edges)
	bool first = true;
	for (TaskNetwork::Tasks::const_iterator it = workList.begin(); it != workList.end(); ++it)
	{
		TaskNetwork::Node* node = *it;
		for (TaskNetwork::Tasks::const_iterator jt = node->successors.begin(); jt != node->successors.end(); ++jt)
		{
			TaskNetwork::Node* succ = *jt;
			if (first)
				first = false;
			else
				os << ", ";
			os << tasksIdsMap[node] << "<" << tasksIdsMap[succ];
		}
	}

	return os;
}


ScopedTaskNetwork::ScopedTaskNetwork() {
}

ScopedTaskNetwork::ScopedTaskNetwork(const Scope& scope, const TaskNetwork& network):
	scope(scope),
	network(network) {
}

ScopedTaskNetwork ScopedTaskNetwork::operator>>(const ScopedTaskNetwork& that) const {
	Scope scope(this->scope);
	Substitution subst = scope.merge(that.scope);
	TaskNetwork tail(that.network);
	tail.substitute(subst);
	return ScopedTaskNetwork(scope, network >> tail);
}
