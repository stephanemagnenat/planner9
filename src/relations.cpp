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
	Variables variables = Variables::identity(arity);

	return ScopedProposition(scope, new Atom(this, variables));
}

bool Relation::check(const Atom& atom, const State& state) const {
	assert(atom.params.size() == arity);
	return state.contains(atom);
}

void Relation::set(const Literal& literal, State& state) const {
	assert(literal.atom.params.size() == arity);
	if(!literal.negated)
		state.insert(literal.atom);
	else
		state.erase(literal.atom);
}

void Relation::groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const {
	OptionalVariables unifier;
	State::AtomsPerRelation::const_iterator it = state.atoms.find(atom.relation);
	if (it != state.atoms.end()) {
		for (State::AtomSet::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
			const Atom& stateAtom = *jt;
			OptionalVariables newUnifier = stateAtom.unify(atom, constantsCount, subst);
			if (newUnifier) {
				if (unifier) {
					return;
				} else {
					unifier = newUnifier;
				}
			}
		}
	}
	if(unifier)
		subst = unifier.get();
}

/// Get variables range (extends it) with the ranges provided by this relation
Relation::VariablesRanges Relation::getRange(const Atom& atom, const State& state, const size_t constantsCount) const {
	VariablesRanges atomRanges;

	for (Variables::const_iterator it = atom.params.begin(); it != atom.params.end(); ++it) {
		const Variable& variable = *it;
		if(variable.index >= constantsCount)
			atomRanges[variable] = VariableRange(constantsCount, false);
	}

	State::AtomsPerRelation::const_iterator it = state.atoms.find(atom.relation);
	if (it != state.atoms.end()) {
		for (State::AtomSet::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
			const Atom& stateAtom = *jt;
			assert(arity == stateAtom.params.size());
			for (size_t j = 0; j < arity; ++j) {
				const Variable& constant = stateAtom.params[j];
				const Variable& variable = atom.params[j];
				assert(constant.index < constantsCount);
				if(variable.index >= constantsCount)
					atomRanges[variable][constant.index] = true;
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
	const Variable& p0 = atom.params[0];
	const Variable& p1 = atom.params[1];
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
	const Variable& p0 = literal.atom.params[0];
	const Variable& p1 = literal.atom.params[1];
	if(p0 == p1) {
		assert(false);
	} else if(p0 < p1) {
		Relation::set(literal, state);
	} else {
		Literal myLiteral(createAtom(p1, p0), literal.negated);
		Relation::set(myLiteral, state);
	}
}

void EquivalentRelation::groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const {
	const Variable& p0 = atom.params[0];
	const Variable& p1 = atom.params[1];

	Atom inverseAtom(createAtom(p1, p0));

	// if present in the state, then not unique
	// FIXME: is it cleaner to use this or atom.relation in the lookup?
	State::AtomsPerRelation::const_iterator it = state.atoms.find(atom.relation);
	if (it != state.atoms.end()) {
		for (State::AtomSet::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
			const Atom& stateAtom = *jt;
			if (stateAtom.unify(atom, constantsCount, subst))
				return;
			if (stateAtom.unify(inverseAtom, constantsCount, subst))
				return;
		}
	}

	// ground with self
	if (p0.index < constantsCount) {
		if (p1.index < constantsCount) {
			return;
		} else {
			if (subst[p1.index] == p1) {
				subst[p1.index] = p0;
			}
		}
	} else {
		if (p1.index < constantsCount) {
			if (subst[p0.index] == p0) {
				subst[p0.index] = p1;
			}
		}
	}
}

Relation::VariablesRanges EquivalentRelation::getRange(const Atom& atom, const State& state, const size_t constantsCount) const {
	const Variable& p0 = atom.params[0];
	const Variable& p1 = atom.params[1];
	VariablesRanges atomRanges;

	if (p0.index < constantsCount) {
		if (p1.index < constantsCount) {
			// we should not be called as simplify() has removed the atom already
			assert(false);
		} else {
			atomRanges = Relation::getRange(atom, state, constantsCount);
			const Atom inverseAtom(createAtom(p1, p0));
			atomRanges[p1] |= Relation::getRange(atom, state, constantsCount)[p1];
			atomRanges[p1][p0.index] = true;
		}
	} else {
		if (p1.index < constantsCount) {
			atomRanges = Relation::getRange(atom, state, constantsCount);
			const Atom inverseAtom(createAtom(p1, p0));
			atomRanges[p0] |= Relation::getRange(atom, state, constantsCount)[p0];
			atomRanges[p0][p1.index] = true;
		} else {
			// do nothing, only variables
		}
	}

	return atomRanges;
}

Atom EquivalentRelation::createAtom(const Variable& p0, const Variable& p1) const {
	Variables params;
	params.reserve(2);
	params.push_back(p0);
	params.push_back(p1);
	return Atom(this, params);
}


EqualityRelation::EqualityRelation():
	Relation("=", 2) {
}

bool EqualityRelation::check(const Atom& atom, const State& /*state*/) const {
	assert(atom.params.size() == 2);
	return atom.params[0] == atom.params[1];
}

void EqualityRelation::set(const Literal& /*literal*/, State& /*state*/) const {
	assert(false);
}

void EqualityRelation::groundIfUnique(const Atom& atom, const State& /*state*/, const size_t constantsCount, Substitution& subst) const {
	// ground with self
	const Variable& p0 = atom.params[0];
	const Variable& p1 = atom.params[1];

	if (p0.index < constantsCount) {
		if (p1.index < constantsCount) {
			return;
		} else {
			if (subst[p1.index] == p1) {
				subst[p1.index] = p0;
			}
		}
	} else {
		if (p1.index < constantsCount) {
			if (subst[p0.index] == p0) {
				subst[p0.index] = p1;
			}
		}
	}
}

Relation::VariablesRanges EqualityRelation::getRange(const Atom& atom, const State& /*state*/, const size_t constantsCount) const {
	const Variable p0 = atom.params[0];
	const Variable p1 = atom.params[1];
	VariablesRanges atomRanges;

	if (p0.index < constantsCount) {
		if (p1.index < constantsCount) {
			// we should not be called as simplify() has removed the atom already
			assert(false);
		} else {
			VariableRange range(constantsCount, false);
			range[p0.index] = true;
			atomRanges[p1] = range;
		}
	} else {
		if (p1.index < constantsCount) {
			VariableRange range(constantsCount, false);
			range[p1.index] = true;
			atomRanges[p0] = range;
		} else {
			// do nothing, only variables
		}
	}

	return atomRanges;
}

EqualityRelation equals;
