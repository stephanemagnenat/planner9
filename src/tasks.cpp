#include "tasks.hpp"
#include "domain.hpp"
#include <iostream>
#include <set>


Task::Task(const Head* head, const Scope::Indices& params, const Tasks& successors):
	head(head),
	params(params),
	successors(successors) {
}

Task Task::substitute(const Scope::Indices& subst) const {
	return Task(head, params.substitute(subst), successors);
}

std::ostream& operator<<(std::ostream& os, const Task& task) {
	return os << task.head->name << "(" << task.params << ")";
}

Scope::Indices Task::getSubstitution(const Scope::Indices& variables, Scope::Index nextVariable) const {
	Scope::Indices subst(params);
	size_t newVariables = variables.size() - params.size();
	for(size_t i = 0; i < newVariables; ++i) {
		subst.push_back(nextVariable + i);
	}
	return variables.substitute(subst);
}


struct Clone {
	Task* operator()(const Task* task) const {
		return new Task(*task);
	}
};
TaskNetwork TaskNetwork::clone() const {
	return rewrite(Clone());
}

struct Substitute {
	Substitute(const Scope::Indices& subst): subst(subst) {}
	Task* operator()(const Task* task) const {
		return new Task(task->substitute(subst));
	}
	const Scope::Indices& subst;
};
TaskNetwork TaskNetwork::substitute(const Scope::Indices& subst) const {
	return rewrite(Substitute(subst));
}

struct Sequence {
	Sequence(const Tasks& successors, size_t& predecessors): successors(successors), predecessors(predecessors) {}
	Task* operator()(const Task* task) const {
		Task* result = new Task(*task);
		if(result->successors.empty()) {
			result->successors.assign(successors.begin(), successors.end());
			++predecessors;
		}
		return result;
	}
	const Tasks& successors;
	size_t& predecessors;
};
TaskNetwork TaskNetwork::operator>>(const TaskNetwork& that) const {
	size_t predecessorsCount = 0;

	TaskNetwork result = rewrite(Sequence(that.first, predecessorsCount));
	if(predecessorsCount == 0) {
		result.first = that.first;
	}

	result.predecessors.insert(that.predecessors.begin(), that.predecessors.end());
	for(Tasks::const_iterator it = that.first.begin(); it != that.first.end(); ++it) {
		result.predecessors[*it] = predecessorsCount;
	}

	return result;
}

TaskNetwork TaskNetwork::erase(Tasks::const_iterator position) const {
	TaskNetwork result;
	result.predecessors = predecessors;

	std::copy(first.begin(), position, std::back_inserter(result.first));

	const Task* task = *position;
	for(Tasks::const_iterator it = task->successors.begin(); it != task->successors.end(); ++it) {
		const Task* successor = *it;
		Predecessors::iterator predecessors = result.predecessors.find(successor);
		--(predecessors->second);
		if(predecessors->second == 0) {
			result.predecessors.erase(predecessors);
			result.first.push_back(successor);
		}
	}

	std::copy(position + 1, first.end(), std::back_inserter(result.first));

	return result;
}

TaskNetwork TaskNetwork::replace(Tasks::const_iterator position, const TaskNetwork& that) const {
	const Task* task = *position;
	size_t predecessorsCount = 0;

	// copy that network
	TaskNetwork result = that.rewrite(Sequence(task->successors, predecessorsCount));
	if(predecessorsCount == 0) {
		result.first = task->successors;
	}

	// copy our first tasks except the replaced one
	result.first.insert(result.first.end(), first.begin(), position);
	result.first.insert(result.first.end(), position + 1, first.end());

	// copy our predecessors counters
	result.predecessors.insert(this->predecessors.begin(), this->predecessors.end());

	// update the predecessors counters of the replaced task's successors
	for(Tasks::const_iterator it = task->successors.begin(); it != task->successors.end(); ++it) {
		Predecessors::iterator succPredCount = result.predecessors.find(*it);
		succPredCount->second += predecessorsCount - 1;
		if(succPredCount->second == 0)
			result.predecessors.erase(succPredCount);
	}

	return result;
}

TaskNetwork TaskNetwork::rewrite(boost::function<Task* (const Task*)> cloner) const {
	TaskNetwork network;

	typedef std::map<const Task*, Task*> Clones;
	Clones clones;

	// clone first row of tasks
	network.first.reserve(first.size());
	for(Tasks::const_iterator it = first.begin(); it != first.end(); ++it) {
		const Task* task = *it;
		Task* clone = cloner(task);
		assert(task);
		assert(clone);
		clones[task] = clone;
		network.first.push_back(clone);
	}

	// clone successors
	for(Predecessors::const_iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
		const Task* task = it->first;
		Task* clone = cloner(task);
		assert(task);
		assert(clone);
		clones[task] = clone;
		network.predecessors[clone] = it->second;
	}

	// replace successors by clones
	for(Clones::const_iterator it = clones.begin(); it != clones.end(); ++it) {
		Task* clone = it->second;
		for(Tasks::iterator jt = clone->successors.begin(); jt != clone->successors.end(); ++jt) {
			Clones::const_iterator found = clones.find(*jt);
			if(found != clones.end()) {
				*jt = found->second;
			}
		}
	}

	return network;
}

std::ostream& operator<<(std::ostream& os, const TaskNetwork& network) {
	typedef std::map<const Task*, size_t> TasksIdsMap;
	TasksIdsMap tasksIdsMap;

	typedef std::set<const Task*> TasksSet;
	TasksSet alreadySeen;
	Tasks workList = network.first;

	// print all tasks (nodes)
	size_t index = 0;
	while(index < workList.size()) {
		const Task* task = workList[index];

		os << index << ":";
		tasksIdsMap[task] = index++;
		os << *task;

		for(Tasks::const_iterator it = task->successors.begin(); it != task->successors.end(); ++it) {
			const Task* succ = *it;
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
	for (Tasks::const_iterator it = workList.begin(); it != workList.end(); ++it)
	{
		const Task* task = *it;
		for (Tasks::const_iterator jt = task->successors.begin(); jt != task->successors.end(); ++jt)
		{
			const Task* succ = *jt;
			if (first)
				first = false;
			else
				os << ", ";
			os << tasksIdsMap[task] << "<" << tasksIdsMap[succ];
		}
	}
	os << std::endl;

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
	Scope::Substitutions substs = scope.merge(that.scope);
	TaskNetwork head = this->network.substitute(substs.first);
	TaskNetwork tail = that.network.substitute(substs.second);
	return ScopedTaskNetwork(scope, head >> tail);
}
