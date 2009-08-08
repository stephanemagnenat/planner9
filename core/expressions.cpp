#include "expressions.hpp"
#include "logic.hpp"
#include "atomimpl.hpp"


ScopedProposition::ScopedProposition():
	proposition(new CNF) {
}

ScopedProposition::ScopedProposition(const ScopedAtomCall<bool>& atom):
	scope(atom.scope),
	proposition(atom.atom->clone()) {
}

ScopedProposition::ScopedProposition(const Scope& scope, const Proposition* proposition):
	scope(scope),
	proposition(proposition) {
}

ScopedProposition::ScopedProposition(const ScopedProposition& proposition):
	scope(proposition.scope),
	proposition(proposition.proposition->clone()) {
}

ScopedProposition::~ScopedProposition() {
	delete proposition;
}

ScopedProposition ScopedProposition::operator!() const {
	return ScopedProposition(scope, new Not(proposition->clone()));
}

ScopedProposition ScopedProposition::operator&&(const ScopedProposition& that) const {
	Scope scope(this->scope);
	Substitution subst = scope.merge(that.scope);
	Proposition* left = this->proposition->clone();
	Proposition* right = that.proposition->clone();
	right->substitute(subst);
	And::Propositions propositions;
	propositions.push_back(left);
	propositions.push_back(right);
	return ScopedProposition(scope, new And(propositions));
}

ScopedProposition ScopedProposition::operator||(const ScopedProposition& that) const {
	Scope scope(this->scope);
	Substitution subst = scope.merge(that.scope);
	Proposition* left = this->proposition->clone();
	Proposition* right = that.proposition->clone();
	right->substitute(subst);
	Or::Propositions propositions;
	propositions.push_back(left);
	propositions.push_back(right);
	return ScopedProposition(scope, new Or(propositions));
}

Proposition ScopedProposition::proposition() {
	return dynamic_cast<const Proposition*>(expression);
}

ScopedProposition True;
