#include "domain.hpp"
#include "logic.hpp"
#include "relations.hpp"
#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <cstdarg>
#include <iostream>


Head::Head(const std::string& name) :
	name(name),
	minCost(0),
	paramsCount(0) {
}

void Head::param(const std::string& name) {
	Scope scope(name);
	Substitutions substs = this->scope.merge(scope);
	variables.substitute(substs.first);
	variables.push_back(substs.second.front());
	++paramsCount;
}

ScopedTaskNetwork Head::operator()(const char* first, ...) const {
	Scope::Names names;
	names.push_back(first);
	va_list vargs;
	va_start(vargs, first);
	for(size_t i = 1; i < paramsCount; ++i)
		names.push_back(va_arg(vargs, const char*));
	va_end(vargs);

	Scope scope(names);
	Variables variables = scope.getVariables(names);

	TaskNetwork network;
	network.first.push_back(new Task(this, variables, Tasks()));

	return ScopedTaskNetwork(scope, network);
}

std::ostream& operator<<(std::ostream& os, const Head& head) {
	return os << head.name << "(" << head.variables << ")";
}


Action::Action(const std::string& name) :
	Head(name) {
}

void Action::pre(const ScopedProposition& precondition) {
	Substitution subst = merge(precondition.scope);
	this->precondition = precondition.proposition->cnf();
	this->precondition.substitute(subst);
}

void Action::add(const ScopedProposition& scopedAtom) {
	effect(scopedAtom, false);
}

void Action::del(const ScopedProposition& scopedAtom) {
	effect(scopedAtom, true);
}

State Action::Effects::apply(const State& state, const Substitution subst) const {
	State newState(state);
	for (const_iterator it = begin(); it != end(); ++it) {
		Literal literal = *it;
		literal.substitute(subst);
		literal.atom.relation->set(literal, newState);
	}
	return newState;
}

void Action::Effects::substitute(const Substitution& subst) {
	for (iterator it = begin(); it != end(); ++it) {
		Literal& literal = *it;
		literal.substitute(subst);
	}
}

void Action::effect(const ScopedProposition& scopedAtom, bool negated) {
	const Atom* originalAtom = dynamic_cast<const Atom*>(scopedAtom.proposition.get());
	assert(originalAtom != 0);
	Atom atom(*originalAtom);
	atom.substitute(merge(scopedAtom.scope));
	effects.push_back(Literal(atom, negated));
}

Substitution Action::merge(const Scope& scope) {
	Substitutions substs = this->scope.merge(scope);

	variables.substitute(substs.first);
	// add new variable indices to our variables
	for(size_t i = 0; i < scope.getSize(); ++i) {
		Variable variable(i);
		if(std::find(variables.begin(), variables.end(), variable) == variables.end()) {
			variables.push_back(variable);
		}
	}

	precondition.substitute(substs.first);

	for(Effects::iterator it = effects.begin(); it != effects.end(); ++it) {
		it->substitute(substs.first);
	}

	return substs.second;
}


Method::Alternative::Alternative(const std::string& name, const Scope& scope, const Variables& variables, const CNF& precondition, const TaskNetwork& tasks, Cost cost):
	name(name),
	scope(scope),
	variables(variables),
	precondition(precondition),
	tasks(tasks),
	cost(cost) {
}

bool Method::Alternative::operator<(const Alternative& that) const {
	return this->cost < that.cost;
}

std::ostream& operator<<(std::ostream& os, const Method::Alternative& alternative) {
	os << Scope::setScope(alternative.scope);
	os << alternative.name << " ";
	os << alternative.cost << std::endl;
	os << alternative.precondition << std::endl;
	os << alternative.tasks;
	return os;
}


Method::Method(const std::string& name) :
	Head(name) {
}

void Method::alternative(const std::string& name, const ScopedProposition& precondition, const ScopedTaskNetwork& decomposition, Cost cost) {
	CNF proposition = precondition.proposition->cnf();
	TaskNetwork network = decomposition.getNetwork().clone();

	// merge precondition and decomposition scopes
	Scope scope(precondition.scope);
	Substitutions substs = scope.merge(decomposition.getScope());

	// rewrite precondition and decomposition with the new scope
	proposition.substitute(substs.first);
	network.substitute(substs.second);

	// merge with parameters scope of this method
	Substitutions substs2 = scope.merge(this->scope);

	// rewrite precondition and decomposition with the new scope
	proposition.substitute(substs2.first);
	network.substitute(substs2.first);

	// compute parameters indices in the alternative scope
	Variables params(getVariables());
	params.substitute(substs2.second);
	Variables variables;
	variables.resize(scope.getSize(), Variable(-1));
	size_t index = 0;
	for(; index < params.size(); ++index) {
		variables[params[index].index] = Variable(index);
	}
	for(Variables::iterator it = variables.begin(); it != variables.end(); ++it) {
		if(*it == Variable(-1)) {
			it->index = index;
			++index;
		}
	}

	Alternative alternative(name, scope, variables, proposition, network, cost);

	// insert the alternative
	Alternatives::iterator position = std::upper_bound(alternatives.begin(), alternatives.end(), alternative);
	if(position == alternatives.begin())
		minCost = cost;
	alternatives.insert(position, alternative);
}

std::ostream& operator<<(std::ostream& os, const Method& method) {
	os << static_cast<const Head&>(method) << std::endl;
	for(Method::Alternatives::const_iterator it = method.alternatives.begin(); it != method.alternatives.end(); ++it) {
		os << *it;
	}
	return os << std::endl;
}
