#include "expressions.hpp"
#include "logic.hpp"

ScopedProposition ScopedLookup<bool>::operator!() const {
	return !ScopedProposition(*this);
}

ScopedProposition ScopedLookup<bool>::operator&&(const ScopedProposition& that) const {
	return ScopedProposition(*this) && that;
}

ScopedProposition ScopedLookup<bool>::operator||(const ScopedProposition& that) const {
	return ScopedProposition(*this) || that;
}

ScopedProposition::ScopedProposition():
	proposition(new CNF) {
}

ScopedProposition::ScopedProposition(const Scope& scope, const Proposition* proposition):
	scope(scope),
	proposition(proposition) {
}

ScopedProposition::ScopedProposition(const ScopedLookup<bool>& scopedLookup):
	scope(scopedLookup.scope),
	proposition(new Atom(scopedLookup.lookup.function, scopedLookup.lookup.params)) {
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


ScopedProposition True;
