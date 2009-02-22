#ifndef DOMAIN_HPP_
#define DOMAIN_HPP_


#include "logic.hpp"
#include "tasks.hpp"
#include "state.hpp"
#include <memory>
#include <string>
#include <vector>

typedef int Cost;

struct Head {

	Head(const std::string& name);

	void param(const std::string& name);

	size_t getFreeVariablesCount() const { return scope.names.size() - paramsCount; }

	ScopedTaskNetwork operator()(const char* first, ...) const;

	const std::string name;
	Cost minCost;

	virtual void dummy() {}

	const Scope& getScope() const { return scope; }
	const Scope::Indices& getVariables() const { return variables; }
	size_t getParamsCount() const { return paramsCount; }

	friend std::ostream& operator<<(std::ostream& os, const Head& head);

protected:

	Scope::Indices merge(const Scope& scope);

	Scope scope;
	Scope::Indices variables;
	size_t paramsCount;

};

/// documentation test
struct Action: Head { // TODO: allow free local variables, like in an alternative

	Action(const std::string& name);

	void pre(const ScopedProposition& precondition);

	void add(const ScopedProposition& atom);

	void del(const ScopedProposition& atom);

	struct Effects:public std::vector<Literal>  {
		State apply(const State& state, const Scope::Indices subst) const;
	};

//	std::vector<std::pair<GroundInstance, Substitution> > groundings();
	const CNF& getPrecondition() const { return precondition; }
	const Effects& getEffects() const { return effects; }

private:

	Scope::Indices merge(const Scope& scope);
	void effect(const ScopedProposition& atom, bool negated);

	CNF precondition;
	Effects effects;

};

struct Method: Head {

	Method(const std::string& name);

	void alternative(const ScopedProposition& precondition, const ScopedTaskNetwork& decomposition = ScopedTaskNetwork(), Cost cost = 1);

	struct Alternative {
		Scope scope;
		Scope::Indices variables;
		CNF precondition;
		TaskNetwork tasks; // TODO: free network's tasks upon delete
		Cost cost;

		Alternative(const Scope& scope, const Scope::Indices& variables, const CNF& precondition, const TaskNetwork& tasks, Cost cost);
		bool operator<(const Alternative& that) const;
		friend std::ostream& operator<<(std::ostream& os, const Alternative& alternative);
		
	};

	typedef std::vector<Alternative> Alternatives;

	Alternatives alternatives;

	friend std::ostream& operator<<(std::ostream& os, const Method& method);

};

#endif // DOMAIN_HPP_
