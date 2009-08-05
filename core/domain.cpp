/*  
    Planner9, an embedded, modular, and distributed HTN planner

    (c) 2009 Martin Voelkle <martin dot voelkle at gmail dot com>

    (c) 2009 Stéphane Magnenat <stephane at magnenat dot net>
    Mobots group (http://mobots.epfl.ch) - LSRO1 - EPFL

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

const Head* Domain::getHead(const std::string& name) const {
	HeadsNamesMap::const_iterator it = headsNamesMap.find(name);
	if (it != headsNamesMap.end())
		return it->second;
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

const Relation* Domain::getRelation(const std::string& name) const {
	RelationsNamesMap::const_iterator it = relationsNamesMap.find(name);
	if (it != relationsNamesMap.end())
		return it->second;
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
	headsNamesMap[head.name] = &head;
	headsVector.push_back(&head);
}

void Domain::registerRelation(const Relation& rel) {
	relationsReverseMap[&rel] = relationsVector.size();
	relationsNamesMap[rel.name] = &rel;
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

	TaskNetwork::Node* node = new TaskNetwork::Node(Task(this, variables));
	
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

void Action::pre() {
	scope.merge(paramsScope); // make sure we have all the params
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

Action::Effect::Effet(const Effect& that) :
	left(that.left),
	right(that.right->clone()) {
}		

Action::Effect::Effet(const Atom::Lookup& left, Atom::Predicate* right) :
	left(left),
	right(right) {
}

Action::Effect::~Effect() {
	delete right;
}

void Action::Effect::substitute(const Substitution& subst) {
	effect.left.substitute(subst);
	effect.right->substitute(subst);
}

State Action::Effects::apply(const State& state, const Substitution subst) const {
	State newState(state);
	for (const_iterator it = begin(); it != end(); ++it) {
		Effect effect(*it);
		effect.substitute(subst);
		right->set(state, newState, left);
	}
	return newState;
}

void Action::Effects::substitute(const Substitution& subst) {
	for (iterator it = begin(); it != end(); ++it) {
		(*it)->substitute(subst);
	}
}

void Action::effect(const ScopedProposition& scopedAtom, bool negated) {
	scope.merge(paramsScope); // make sure we have all the params
	const Atom* originalAtom = boost::polymorphic_downcast<const Atom*>(scopedAtom.proposition);
	const Atom::Lookup* originalLookup = boost::polymorphic_downcast<const Atom::Lookup*>(originalAtom->predicate);
	Atom::Lookup lookup(*originalLookup);
	lookup.substitute(scope.merge(scopedAtom.scope));
	if (negated)
		effects.push_back(Effect(lookup, new Call(ReturnFalse)));
	else
		effects.push_back(Effect(lookup, new Call(ReturnTrue)));
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
