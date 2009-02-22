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
	Scope::Substitutions substs = this->scope.merge(scope);
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
	Scope::Indices indices = scope.getIndices(names);

	TaskNetwork network;
	network.first.push_back(new Task(this, indices, Tasks()));

	return ScopedTaskNetwork(scope, network);
}

std::ostream& operator<<(std::ostream& os, const Head& head) {
	Scope::Names names = head.scope.getNames(head.variables);
	return os << head.name << "(" << boost::algorithm::join(names, ", ") << ")";
}


Action::Action(const std::string& name) :
	Head(name) {
}

void Action::pre(const ScopedProposition& precondition) {
	Scope::Indices subst = merge(precondition.scope);
	this->precondition = precondition.proposition->cnf();
	this->precondition.substitute(subst);
}

void Action::add(const ScopedProposition& scopedAtom) {
	effect(scopedAtom, false);
}

void Action::del(const ScopedProposition& scopedAtom) {
	effect(scopedAtom, true);
}

State Action::Effects::apply(const State& state, const Scope::Indices subst) const {
	State newState(state);
	for (const_iterator it = begin(); it != end(); ++it) {
		Literal literal = *it;
		literal.substitute(subst);
		literal.atom.relation->set(literal, newState);
	}
	return newState;
}

void Action::effect(const ScopedProposition& scopedAtom, bool negated) {
	const Atom* originalAtom = dynamic_cast<const Atom*>(scopedAtom.proposition.get());
	assert(originalAtom != 0);
	Atom atom(*originalAtom);
	atom.substitute(merge(scopedAtom.scope));
	effects.push_back(Literal(atom, negated));
}

Scope::Indices Action::merge(const Scope& scope) {
	Scope::Substitutions substs = this->scope.merge(scope);

	variables.substitute(substs.first);
	// add new variable indices to our variables
	for(Scope::Index i = 0; i < scope.getSize(); ++i) {
		if(std::find(variables.begin(), variables.end(), i) == variables.end()) {
			variables.push_back(i);
		}
	}

	precondition.substitute(substs.first);

	for(Effects::iterator it = effects.begin(); it != effects.end(); ++it) {
		it->substitute(substs.first);
	}

	return substs.second;
}


Method::Alternative::Alternative(const Scope& scope, const Scope::Indices& variables, const CNF& precondition, const TaskNetwork& tasks, Cost cost):
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
	os << alternative.cost << std::endl;
	os << alternative.precondition << std::endl;
	os << alternative.tasks;
	return os;
}


Method::Method(const std::string& name) :
	Head(name) {
}

void Method::alternative(const ScopedProposition& precondition, const ScopedTaskNetwork& decomposition, Cost cost) {
	CNF proposition = precondition.proposition->cnf();
	const TaskNetwork& network = decomposition.getNetwork();

	// merge precondition and decomposition scopes
	Scope scope(precondition.scope);
	Scope::Substitutions substs = scope.merge(decomposition.getScope());

	// rewrite precondition and decomposition with the new scope
	proposition.substitute(substs.first);
	TaskNetwork network2 = network.substitute(substs.second);

	// merge with parameters scope of this method
	Scope::Substitutions substs2 = scope.merge(this->scope);

	// rewrite precondition and decomposition with the new scope
	proposition.substitute(substs2.first);
	TaskNetwork network3 = network2.substitute(substs2.first); // TODO: mutating substitute

	// compute parameters indices in the alternative scope
	// TODO: FIXME: this substitution is wrong
	Scope::Indices subst = getVariables().substitute(substs2.second);
	for(Scope::Index i = 0; i < scope.getSize(); ++i) {
		if(std::find(subst.begin(), subst.end(), i) == subst.end()) {
			subst.push_back(i);
		}
	}

	Alternative alternative(scope, subst, proposition, network3, cost);

	// insert the alternative
	Alternatives::iterator position = std::lower_bound(alternatives.begin(), alternatives.end(), alternative);
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
