#include "domain.hpp"
#include "logic.hpp"
#include "relations.hpp"
#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <cstdarg>
#include <iostream>

Domain::Domain() {
	registerRelation(equals);
}

const Head* Domain::getHead(size_t index) const {
	if (index < headsVector.size())
		return headsVector[index];
	else
		return 0;
}

size_t Domain::getHeadIndex(const Head* head) const {
	HeadsReverseMap::const_iterator it = headsReverseMap.find(head);
	if (it != headsReverseMap.end())
		return it->second;
	else
		return (size_t)-1;
}

const Relation* Domain::getRelation(size_t index) const {
	if (index < relationsVector.size())
		return relationsVector[index];
	else
		return 0;
}

size_t Domain::getRelationIndex(const Relation* rel) const {
	RelationsReverseMap::const_iterator it = relationsReverseMap.find(rel);
	if (it != relationsReverseMap.end())
		return it->second;
	else
		return (size_t)-1;
}

void Domain::registerHead(const Head& head) {
	headsReverseMap[&head] = headsVector.size();
	headsVector.push_back(&head);
}

void Domain::registerRelation(const Relation& rel) {
	relationsReverseMap[&rel] = relationsVector.size();
	relationsVector.push_back(&rel);
}
	

Head::Head(Domain* domain, const std::string& name) :
	name(name),
	minCost(0) {
	domain->registerHead(*this);
}

void Head::param(const std::string& name) {
	Scope scope(name);
	const size_t scopeSize(paramsScope.getSize());
	paramsScope.merge(scope);
	if (paramsScope.getSize() != scopeSize + 1) {
		std::cerr << this->name << ": Each parameter must have a different name" << std::endl;
		abort();
	}
}

ScopedTaskNetwork Head::operator()(const char* first, ...) const {
	Scope::Names names;
	names.push_back(first);
	va_list vargs;
	va_start(vargs, first);
	for(size_t i = 1; i < getParamsCount(); ++i)
		names.push_back(va_arg(vargs, const char*));
	va_end(vargs);

	if (names.size() != getParamsCount())
	{
		std::cerr << "warning task " << name << " has only" << names.size() << " parameters instead of " << getParamsCount() << " as declared!" << std::endl;
		abort();
		// FIXME: manage pre/circular definitions correctly
	}

	Scope scope(names);
	Variables variables = scope.getVariables(names);

	TaskNetwork::Node* node = new TaskNetwork::Node(Task(this, indices));
	
	TaskNetwork network;
	network.first.push_back(node);

	return ScopedTaskNetwork(scope, network);
}

std::ostream& operator<<(std::ostream& os, const Head& head) {
	return os << head.name << "(" << head.paramsScope << ")";
}


Action::Action(Domain* domain, const std::string& name) :
	Head(domain, name) {
}

void Action::pre(const ScopedProposition& precondition) {
	scope.merge(paramsScope); // make sure we have all the params
	Substitution subst = scope.merge(precondition.scope);
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
	scope.merge(paramsScope); // make sure we have all the params
	const Atom* originalAtom = dynamic_cast<const Atom*>(scopedAtom.proposition);
	assert(originalAtom != 0);
	Atom atom(*originalAtom);
	atom.substitute(scope.merge(scopedAtom.scope));
	effects.push_back(Literal(atom, negated));
}


Method::Alternative::Alternative(const std::string& name, const Scope& scope, const CNF& precondition, const TaskNetwork& tasks, Cost cost):
	name(name),
	scope(scope),
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


Method::Method(Domain* domain, const std::string& name) :
	Head(domain, name) {
}

void Method::alternative(const std::string& name, const ScopedProposition& precondition, const ScopedTaskNetwork& decomposition, Cost cost) {
	CNF proposition = precondition.proposition->cnf();
	TaskNetwork network(decomposition.getNetwork());

	Scope scope(paramsScope);

	// rewrite precondition for the alternative
	Substitution preconditionSubst = scope.merge(precondition.scope);
	proposition.substitute(preconditionSubst);

	// rewrite decomposition for the alternative
	Substitution decompositionSubst = scope.merge(decomposition.getScope());
	network.substitute(decompositionSubst);

	// hack to have later alternatives more expensives, simulates more a depth-first search
	//cost = cost << (alternatives.size()*1);

	Alternative alternative(name, scope, proposition, network, cost);

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
