#include "relations.hpp"
#include "state.hpp"
#include "domain.hpp"
#include <cassert>

Variables createParams(const Variable& p0, const Variable& p1) const {
	Variables params;
	params.reserve(2);
	params.push_back(p0);
	params.push_back(p1);
	return params;
}

AbstractFunction::AbstractFunction(Domain* domain, const std::string& name, size_t arity):
	name(name),
	arity(arity) {
	if (domain)
		domain->registerRelation(*this);
}

AbstractFunction::VariablesRanges AbstractFunction::getRange(const Variables& params, const State& state, const size_t constantsCount) const {
	// return full range in general case, as no entry means zero element of type
	return VariablesRanges();
}

void AbstractFunction::groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const {
	// do nothing in the general case
}


template<typename CoDomain>
CoDomain Function::get(const Variables& params, const State& state) const {
	assert(params.size() == arity);
	State::Functions::const_iterator it = state.functions.find(this);
	if (it == functions.end())
		return CoDomain();

	typedef FunctionState<Function<CoDomain> >::Values Values;
	Values::const_iterator jt = it->second.values.find(params);
	if (jt == it->second.values.end())
		return CoDomain();
		
	return jt->second;
}

template<typename CoDomain>
void Function::set(const Variables& params, State& state, const CoDomain& value) const {
	assert(params.size() == arity);
	
	typedef FunctionState<Function<CoDomain> > FunctionState;
	if (value != CoDomain()) {
		FunctionState*& functionState(functions[this]);
		if (!functionState)
			functionState = new FunctionState();
		functionState->values[params] = value;
	} else {
		State::Functions::iterator it = state.functions.find(this);
		if (it == state.functions.end())
			return;
		it->values.erase(params);
		if (it->values.empty())	{
			delete *it;
			functions.erase(it);
		}
	}
}

template<typename CoDomain>
SymmetricFunction::SymmetricFunction(Domain* domain, const std::string& name):
	Relation(domain, name, 2) {
}

template<typename CoDomain>
bool SymmetricFunction::get(const Variables& params, const State& state) const {
	assert(params.size() == 2);
	const Variable& p0 = params[0];
	const Variable& p1 = params[1];
	if(p0 <= p1) {
		return Function<CoDomain>::get(params, state);
	} else {
		return Function<CoDomain>::get(createParams(p1, p0), state);
	}
}

template<typename CoDomain>
void SymmetricFunction::set(const Variables& params, State& state, const CoDomain& value) const {
	assert(params.size() == 2);
	const Variable& p0 = literal.atom.params[0];
	const Variable& p1 = literal.atom.params[1];
	if(p0 <= p1) {
		Function<CoDomain>::set(params, state, value);
	} else {
		Function<CoDomain>::set(createAtom(p1, p0), state, value);
	}
}

OptionalVariables unify(const Variables& stateParams, const Variables& params, const size_t constantsCount, const Substitution& subst) const {
	Substitution unifyingSubst(subst);
	assert(stateParams.size() == params.size());
	for (size_t i = 0; i < stateParams.size(); ++i) {
		const Variable& stateVariable = stateParams[i];
		const Variable& variable = params[i];
		if (variable.index < constantsCount) {
			if (variable != stateVariable)
				return false;
		} else {
			assert(variable.index < unifyingSubst.size());
			Variable& substitutionVariable = unifyingSubst[variable.index];
			if (substitutionVariable != variable) {
				if (substitutionVariable != stateVariable)
					return false;
			} else {
				substitutionVariable = stateVariable;
			}
		}
	}
	return unifyingSubst;
}


void Relation::groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const {
	OptionalVariables unifier;
	State::Functions::const_iterator it = state.functions.find(function);
	if (it != state.functions.end()) {
		typedef typename State::FunctionState<bool> RelationState;
		const RelationState* relationState(boost::polymorphic_downcast<const RelationState*>(*it));
		for (RelationState::Values::const_iterator jt = relationState.values.begin(); jt != relationState.values.end(); ++jt) {
			const Variables& stateParams(*jt);
			OptionalVariables newUnifier = unify(statePredicate, params, constantsCount, subst);
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
Relation::VariablesRanges Relation::getRange(const Variables& params, const State& state, const size_t constantsCount) const {
	VariablesRanges atomRanges;

	for (Variables::const_iterator it = params.begin(); it != params.end(); ++it) {
		const Variable& variable = *it;
		if(variable.index >= constantsCount)
			atomRanges[variable] = VariableRange(constantsCount, false);
	}

	State::Functions::const_iterator it = state.functions.find(function);
	if (it != state.functions.end()) {
		typedef typename State::FunctionState<bool> RelationState;
		const RelationState* relationState(boost::polymorphic_downcast<const RelationState*>(*it));
		for (RelationState::Values::const_iterator jt = relationState.values.begin(); jt != relationState.values.end(); ++jt) {
			const Variables& stateParams(*jt);
			assert(arity == stateParams.size());
			for (size_t j = 0; j < arity; ++j) {
				const Variable& constant = stateParams[j];
				const Variable& variable = params[j];
				assert(constant.index < constantsCount);
				if(variable.index >= constantsCount)
					atomRanges[variable][constant.index] = true;
			}
		}
	}
	return atomRanges;
}

bool EquivalentRelation::get(const Variables& params, const State& state) const {
	assert(params.size() == 2);
	const Variable& p0 = params[0];
	const Variable& p1 = params[1];
	if (p0 == p1) {
		return true;
	} else if (p0 < p1) {
		return Function<bool>::get(params, state);
	} else {
		return Function<bool>::get(createParams(p1, p0), state);
	}
}

void EquivalentRelation::set(const Variables& params, State& state, const bool& value) const {
	assert(params.size() == 2);
	const Variable& p0 = params[0];
	const Variable& p1 = params[1];
	if(p0 == p1) {
		assert(false);
	} else if(p0 < p1) {
		Function<bool>::set(params, state, value);
	} else {
		Function<bool>::set(createParams(p1, p0), state, value);
	}
}

void EquivalentRelation::groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const {
	const Variable& p0 = atom.params[0];
	const Variable& p1 = atom.params[1];

	Variables inverseParams(createParams(p1, p0));

	// if present in the state, then not unique
	State::Functions::const_iterator it = state.functions.find(function);
	if (it != state.functions.end()) {
		typedef typename State::FunctionState<bool> RelationState;
		const RelationState* relationState(boost::polymorphic_downcast<const RelationState*>(*it));
		for (RelationState::Values::const_iterator jt = relationState.values.begin(); jt != relationState.values.end(); ++jt) {
			const Variables& stateParams(*jt);
			if (unify(stateParams, params, constanstCount, subst))
				return;
			if (unify(stateParams, inverseParams, constanstCount, subst))
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

Relation::VariablesRanges EquivalentRelation::getRange(const Variables& params, const State& state, const size_t constantsCount) const {
	const Variable& p0 = params[0];
	const Variable& p1 = params[1];
	VariablesRanges atomRanges;

	if (p0.index < constantsCount) {
		if (p1.index < constantsCount) {
			// we should not be called as simplify() has removed the atom already
			assert(false);
		} else {
			atomRanges = Relation::getRange(params, state, constantsCount);
			const Variables inverseParams(createParams(p1, p0));
			atomRanges[p1] |= Relation::getRange(inverseParams, state, constantsCount)[p1];
			atomRanges[p1][p0.index] = true;
		}
	} else {
		if (p1.index < constantsCount) {
			atomRanges = Relation::getRange(params, state, constantsCount);
			const Variables inverseParams(createParams(p1, p0));
			// TODO: check this, maybe there was a bug here before
			atomRanges[p0] |= Relation::getRange(inverseParams, state, constantsCount)[p0];
			atomRanges[p0][p1.index] = true;
		} else {
			// do nothing, only variables
		}
	}

	return atomRanges;
}



EqualityRelation::EqualityRelation():
	Relation(0, "=", 2) {
}

bool EqualityRelation::get(const Variables& params, const State& /*state*/) const {
	assert(params.size() == 2);
	return params[0] == params[1];
}

void EqualityRelation::set(const Variables& params, State& /*state*/, const bool& /*value*/) const; {
	assert(false);
}

void EqualityRelation::groundIfUnique(const Variables& params, const State& /*state*/, const size_t constantsCount, Substitution& subst) const {
	// ground with self
	const Variable& p0 = params[0];
	const Variable& p1 = params[1];

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

Relation::VariablesRanges EqualityRelation::getRange(const Variables& params, const State& /*state*/, const size_t constantsCount) const {
	const Variable p0 = params[0];
	const Variable p1 = params[1];
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
