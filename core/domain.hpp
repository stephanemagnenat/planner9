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
	virtual ~Head() {}

	void param(const std::string& name);

	ScopedTaskNetwork operator()(const char* first, ...) const;

	const std::string name;
	Cost minCost;

	const Scope& getParamsScope() const { return paramsScope; }
	size_t getParamsCount() const { return paramsScope.getSize(); }

	friend std::ostream& operator<<(std::ostream& os, const Head& head);

protected:

	Scope paramsScope;

};

/// documentation test
struct Action: Head {

	Action(const std::string& name);

	void pre(const ScopedProposition& precondition);

	void add(const ScopedProposition& atom);

	void del(const ScopedProposition& atom);

	struct Effects:public std::vector<Literal>  {
		State apply(const State& state, const Scope::Indices subst) const;
		void substitute(const Scope::Indices& subst);
	};

	const Scope& getScope() const { return scope; }
//	std::vector<std::pair<GroundInstance, Substitution> > groundings();
	const CNF& getPrecondition() const { return precondition; }
	const Effects& getEffects() const { return effects; }

private:

	void effect(const ScopedProposition& atom, bool negated);

	Scope scope;
	CNF precondition;
	Effects effects;

};

struct Method: Head {

	Method(const std::string& name);

	void alternative(const std::string& name, const ScopedProposition& precondition, const ScopedTaskNetwork& decomposition = ScopedTaskNetwork(), Cost cost = 1);

	struct Alternative {
		std::string name;
		Scope scope;
		CNF precondition;
		TaskNetwork tasks; // TODO: free network's tasks upon delete
		Cost cost;

		Alternative(const std::string& name, const Scope& scope, const CNF& precondition, const TaskNetwork& tasks, Cost cost);
		bool operator<(const Alternative& that) const;
		friend std::ostream& operator<<(std::ostream& os, const Alternative& alternative);
		
	};

	typedef std::vector<Alternative> Alternatives;

	Alternatives alternatives;

	friend std::ostream& operator<<(std::ostream& os, const Method& method);

};

#endif // DOMAIN_HPP_
