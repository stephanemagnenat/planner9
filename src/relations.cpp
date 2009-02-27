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
		Scope::Indices indices;
		indices.reserve(2);
		indices.push_back(p1);
		indices.push_back(p0);
		Atom myAtom(this, indices);
		return Relation::check(myAtom, state);
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
		Scope::Indices indices;
		indices.reserve(2);
		indices.push_back(p1);
		indices.push_back(p0);
		Literal myLiteral(Atom(this, indices), literal.negated);
		Relation::set(myLiteral, state);
	}
}

void EquivalentRelation::getRange(const State& state, VariablesRange& variablesRange) const {
	assert(variablesRange.size() == 2);
	for (State::const_iterator it = state.begin(); it != state.end(); ++it) {
		const Atom& atom = *it;
		if (atom.relation == this) {
			const size_t maxRange = std::min(variablesRange[0].size(), variablesRange[1].size());
			assert(atom.params[0] < maxRange);
			assert(atom.params[1] < maxRange);
			variablesRange[0][atom.params[0]] = true;
			variablesRange[0][atom.params[1]] = true;
			variablesRange[1][atom.params[0]] = true;
			variablesRange[1][atom.params[1]] = true;
		}
	}
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

void EqualityRelation::getRange(const State& state, VariablesRange& variablesRange) const {
	// do nothing as the state cannot have a isSame relation
}

EqualityRelation equals;
