#include "relations.hpp"
#include "state.hpp"
#include <cstdarg>
#include <cassert>


Relation::Relation(const std::string& name, size_t arity):
	name(name),
	arity(arity) {
}

ScopedProposition Relation::operator()(const char* first, ...) {
	Scope::Names names;
	names.push_back(first);
	va_list vargs;
	va_start(vargs, first);
	for (size_t i = 1; i < arity; ++i)
		names.push_back(va_arg(vargs, const char*));
	va_end(vargs);

	Scope scope(names);
	Scope::Indices indices = scope.getIndices(names);

	std::auto_ptr<const Proposition> atom(new Atom(this, indices));
	return ScopedProposition(scope, atom);
}

bool Relation::check(const Atom& atom, const State& state) const {
	assert(atom.params.size() == arity);
	return state.find(atom) != state.end();
}

void Relation::set(const Literal& literal, State& state) const {
	assert(literal.atom.params.size() == arity);
	if(!literal.negated)
		state.insert(literal.atom);
	else
		state.erase(literal.atom);
}

void Relation::groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Scope::Indices& subst) const {
	Scope::OptionalIndices unifier;
	for (State::const_iterator it = state.begin(); it != state.end(); ++it) {
		Scope::OptionalIndices newUnifier = it->unify(atom, constantsCount, subst);
		if (newUnifier) {
			if (unifier) {
				return;
			} else {
				unifier = newUnifier;
			}
		}	
	}
	subst = unifier.get();
}

void Relation::getRange(const State& state, VariablesRange& variablesRange) const {
	assert(variablesRange.size() == arity);
	for (State::const_iterator it = state.begin(); it != state.end(); ++it) {
		const Atom& atom = *it;
		if (atom.relation == this) {
			for (size_t j = 0; j < atom.params.size(); ++j) {
				Scope::Index index = atom.params[j];
				assert(index < variablesRange[j].size());
				variablesRange[j][index] = true;
			}
		}
	}
}

EquivalentRelation::EquivalentRelation(const std::string& name):
	Relation(name, 2) {
}

bool EquivalentRelation::check(const Atom& atom, const State& state) const {
	assert(atom.params.size() == 2);
	Scope::Index p0 = atom.params[0];
	Scope::Index p1 = atom.params[1];
	if(p0 == p1) {
		return true;
	} else if(p0 < p1) {
		return Relation::check(atom, state);
	} else {
		Atom inverseAtom(createAtom(p1, p0));
		return Relation::check(inverseAtom, state);
	}
}

void EquivalentRelation::set(const Literal& literal, State& state) const {
	assert(literal.atom.params.size() == 2);
	Scope::Index p0 = literal.atom.params[0];
	Scope::Index p1 = literal.atom.params[1];
	if(p0 == p1) {
		assert(false);
	} else if(p0 < p1) {
		Relation::set(literal, state);
	} else {
		Literal myLiteral(createAtom(p1, p0), literal.negated);
		Relation::set(myLiteral, state);
	}
}

void EquivalentRelation::groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Scope::Indices& subst) const {
	Scope::Index p0 = atom.params[0];
	Scope::Index p1 = atom.params[1];
	
	Atom inverseAtom(createAtom(p1, p0));
	
	// if present in the state, then not unique
	for (State::const_iterator it = state.begin(); it != state.end(); ++it) {
		if (it->unify(atom, constantsCount, subst))
			return;
		if (it->unify(inverseAtom, constantsCount, subst))
			return;
	}
	
	// ground with self
	if (p0 < constantsCount) {
		if (p1 < constantsCount) {
			return;
		} else {
			if (subst[p1] == p1) {
				subst[p1] = p0;
			}
		}
	} else {
		if (p1 < constantsCount) {
			if (subst[p0] == p0) {
				subst[p0] = p1;
			}
		}
	}
}

void EquivalentRelation::getRange(const State& state, VariablesRange& variablesRange) const {
	// an equivalent relation always have the full range, because of the reflexivity
	// we forbid the call in the caller
	assert(false);
}

Atom EquivalentRelation::createAtom(const Scope::Index p0, const Scope::Index p1) const {
	Scope::Indices params;
	params.reserve(2);
	params.push_back(p0);
	params.push_back(p1);
	return Atom(this, params);
}


EqualityRelation::EqualityRelation():
	Relation("=", 2) {
}

bool EqualityRelation::check(const Atom& atom, const State& state) const {
	assert(atom.params.size() == 2);
	return atom.params[0] == atom.params[1];
}

void EqualityRelation::set(const Literal& literal, State& state) const {
	assert(false);
}

void EqualityRelation::groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Scope::Indices& subst) const {
	// ground with self
	Scope::Index p0 = atom.params[0];
	Scope::Index p1 = atom.params[1];
	
	if (p0 < constantsCount) {
		if (p1 < constantsCount) {
			return;
		} else {
			if (subst[p1] == p1) {
				subst[p1] = p0;
			}
		}
	} else {
		if (p1 < constantsCount) {
			if (subst[p0] == p0) {
				subst[p0] = p1;
			}
		}
	}
}

void EqualityRelation::getRange(const State& state, VariablesRange& variablesRange) const {
	// do nothing as the state cannot have a isSame relation
	assert(false);
}

EqualityRelation equals;
