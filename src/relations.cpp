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

/// Get variables range (extends it) with the ranges provided by this relation
Relation::VariablesRanges Relation::getRange(const Atom& atom, const State& state, const size_t constantsCount) const {
	VariablesRanges atomRanges;

	for (Scope::Indices::const_iterator it = atom.params.begin(); it != atom.params.end(); ++it) {
		const Scope::Index index = *it;
		if(index >= constantsCount)
			atomRanges[index] = VariableRange(constantsCount, false);
	}

	for (State::const_iterator it = state.begin(); it != state.end(); ++it) {
		const Atom& stateAtom = *it;
		if (stateAtom.relation == this) {
			assert(arity == stateAtom.params.size());
			for (size_t j = 0; j < arity; ++j) {
				Scope::Index constantIndex = stateAtom.params[j];
				Scope::Index variableIndex = atom.params[j];
				assert(constantIndex < constantsCount);
				if(variableIndex >= constantsCount)
					atomRanges[variableIndex][constantIndex] = true;
			}
		}
	}
	return atomRanges;
}

EquivalentRelation::EquivalentRelation(const std::string& name):
	Relation(name, 2) {
}

bool EquivalentRelation::check(const Atom& atom, const State& state) const {
	assert(atom.params.size() == 2);
	const Scope::Index p0 = atom.params[0];
	const Scope::Index p1 = atom.params[1];
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
	const Scope::Index p0 = literal.atom.params[0];
	const Scope::Index p1 = literal.atom.params[1];
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
	const Scope::Index p0 = atom.params[0];
	const Scope::Index p1 = atom.params[1];
	
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

Relation::VariablesRanges EquivalentRelation::getRange(const Atom& atom, const State& state, const size_t constantsCount) const {
	const Scope::Index p0 = atom.params[0];
	const Scope::Index p1 = atom.params[1];
	VariablesRanges atomRanges;

	if (p0 < constantsCount) {
		if (p1 < constantsCount) {
			// we should not be called as simplify() has removed the atom already
			assert(false);
		} else {
			atomRanges = Relation::getRange(atom, state, constantsCount);
			const Atom inverseAtom(createAtom(p1, p0));
			atomRanges[p1] |= Relation::getRange(atom, state, constantsCount)[p1];
			atomRanges[p1][p0] = true;
		}
	} else {
		if (p1 < constantsCount) {
			atomRanges = Relation::getRange(atom, state, constantsCount);
			const Atom inverseAtom(createAtom(p1, p0));
			atomRanges[p0] |= Relation::getRange(atom, state, constantsCount)[p0];
			atomRanges[p0][p1] = true;
		} else {
			// do nothing, only variables
		}
	}

	return atomRanges;
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
	const Scope::Index p0 = atom.params[0];
	const Scope::Index p1 = atom.params[1];
	
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

Relation::VariablesRanges EqualityRelation::getRange(const Atom& atom, const State& state, const size_t constantsCount) const {
	const Scope::Index p0 = atom.params[0];
	const Scope::Index p1 = atom.params[1];
	VariablesRanges atomRanges;

	if (p0 < constantsCount) {
		if (p1 < constantsCount) {
			// we should not be called as simplify() has removed the atom already
			assert(false);
		} else {
			VariableRange range(constantsCount, false);
			range[p0] = true;
			atomRanges[p1] = range;
		}
	} else {
		if (p1 < constantsCount) {
			VariableRange range(constantsCount, false);
			range[p1] = true;
			atomRanges[p0] = range;
		} else {
			// do nothing, only variables
		}
	}

	return atomRanges;
}

EqualityRelation equals;
