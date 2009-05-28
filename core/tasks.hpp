#ifndef TASKS_HPP_
#define TASKS_HPP_


#include "scope.hpp"
#include <map>
#include <vector>


struct Head;
struct Task;


struct Task {

	Task(const Head* head, const Scope::Indices& params);

	void substitute(const Scope::Indices& subst);

	const Head* head;
	Scope::Indices params;

	friend std::ostream& operator<<(std::ostream& os, const Task& task);

	Scope::Indices getSubstitution(const size_t taskScopeSize, const Scope::Index nextVariable) const;

};

struct TaskNetwork {
	
	TaskNetwork();
	TaskNetwork(const TaskNetwork& that);
	TaskNetwork& operator=(const TaskNetwork& that);
	~TaskNetwork();

	void substitute(const Scope::Indices& subst);
	TaskNetwork operator>>(const TaskNetwork& that) const;

	struct Node;
	typedef std::vector<Node*> Tasks;
	
	struct Node {
		Node(const Task& task);
		Task task;
		Tasks successors;
	};
	typedef std::map<Node*, size_t> Predecessors;

	void erase(size_t position);
	void replace(size_t position, const TaskNetwork& that);

	Tasks first;
	Predecessors predecessors;

private:

	friend std::ostream& operator<<(std::ostream& os, const TaskNetwork& network);

};


struct ScopedTaskNetwork {

	ScopedTaskNetwork();
	ScopedTaskNetwork(const Scope& scope, const TaskNetwork& network);

	ScopedTaskNetwork operator>>(const ScopedTaskNetwork& that) const;
	//ScopedTaskNetwork operator||(const ScopedTaskNetwork& that) const;

	const Scope& getScope() const { return scope; }
	const TaskNetwork& getNetwork() const { return network; }

private:
	Scope scope;
	TaskNetwork network;

};

#endif // TASKS_HPP_
