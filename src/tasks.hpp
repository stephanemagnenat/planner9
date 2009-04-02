#ifndef TASKS_HPP_
#define TASKS_HPP_


#include "variable.hpp"
#include "scope.hpp"
#include <boost/function.hpp>
#include <map>
#include <vector>


struct Head;
struct Task;


typedef std::vector<const Task*> Tasks;


struct Task {

	Task(const Head* head, const Variables& params, const Tasks& successors);

	void substitute(const Substitution& subst);

	const Head* head;
	Variables params;
	Tasks successors;

	friend std::ostream& operator<<(std::ostream& os, const Task& task);

	Substitution getSubstitution(const Variables& variables, Variable::Index nextVariableIndex) const;

};

struct TaskNetwork {

	TaskNetwork clone() const;
	void substitute(const Substitution& subst);
	TaskNetwork operator>>(const TaskNetwork& that) const;

	TaskNetwork erase(Tasks::const_iterator position) const;
	TaskNetwork replace(Tasks::const_iterator position, const TaskNetwork& that) const;

	size_t getSize() const { return first.size() + predecessors.size(); }

	typedef std::map<const Task*, size_t> Predecessors;

	Tasks first;
	Predecessors predecessors;

private:

	TaskNetwork rewrite(boost::function<Task* (const Task*)> cloner) const;

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
	TaskNetwork network; // TODO: free network's tasks upon delete

};

#endif // TASKS_HPP_
