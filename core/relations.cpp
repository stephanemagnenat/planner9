#include "relations.hpp"
#include "state.hpp"
#include <cassert>
#include <boost/cast.hpp>

Variables createParams(const Variable& p0, const Variable& p1) {
	Variables params;
	params.reserve(2);
	params.push_back(p0);
	params.push_back(p1);
	return params;
}

AbstractFunction::AbstractFunction(const std::string& name, size_t arity, bool deleteWithDomain):
	name(name),
	arity(arity),
	deleteWithDomain(deleteWithDomain) {
}

VariablesRanges AbstractFunction::getRange(const Variables& params, const State& state, const size_t constantsCount) const {
	// return full range in general case, as no entry means zero element of type
	return VariablesRanges();
}

void AbstractFunction::groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const {
	// do nothing in the general case
}


Relation::Relation(const std::string& name, size_t arity):
	Function<bool>(name, arity) {
}

OptionalVariables unify(const Variables& stateParams, const Variables& params, const size_t constantsCount, const Substitution& subst) {
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
	typedef State::FunctionState<bool> RelationState;
	typedef RelationState::Values Values;
	
	OptionalVariables unifier;
	State::Functions::const_iterator it = state.functions.find(this);
	if (it != state.functions.end()) {
		
		const RelationState* relationState(boost::polymorphic_downcast<const RelationState*>(it->second));
		for (Values::const_iterator jt = relationState->values.begin(); jt != relationState->values.end(); ++jt) {
			const Variables& stateParams(jt->first);
			OptionalVariables newUnifier = unify(stateParams, params, constantsCount, subst);
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
VariablesRanges Relation::getRange(const Variables& params, const State& state, const size_t constantsCount) const {
	typedef State::FunctionState<bool> RelationState;
	typedef RelationState::Values Values;
	
	VariablesRanges atomRanges;

	for (Variables::const_iterator it = params.begin(); it != params.end(); ++it) {
		const Variable& variable = *it;
		if(variable.index >= constantsCount)
			atomRanges[variable] = VariableRange(constantsCount, false);
	}

	State::Functions::const_iterator it = state.functions.find(this);
	if (it != state.functions.end()) {
		const RelationState* relationState(boost::polymorphic_downcast<const RelationState*>(it->second));
		for (Values::const_iterator jt = relationState->values.begin(); jt != relationState->values.end(); ++jt) {
			const Variables& stateParams(jt->first);
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

EquivalentRelation::EquivalentRelation(const std::string& name):
	Relation(name, 2) {
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
	typedef State::FunctionState<bool> RelationState;
	typedef RelationState::Values Values;
	
	const Variable& p0 = params[0];
	const Variable& p1 = params[1];

	Variables inverseParams(createParams(p1, p0));

	// if present in the state, then not unique
	State::Functions::const_iterator it = state.functions.find(this);
	if (it != state.functions.end()) {
		const RelationState* relationState(boost::polymorphic_downcast<const RelationState*>(it->second));
		for (Values::const_iterator jt = relationState->values.begin(); jt != relationState->values.end(); ++jt) {
			const Variables& stateParams(jt->first);
			if (unify(stateParams, params, constantsCount, subst))
				return;
			if (unify(stateParams, inverseParams, constantsCount, subst))
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

VariablesRanges EquivalentRelation::getRange(const Variables& params, const State& state, const size_t constantsCount) const {
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
	Relation("=", 2) {
}

bool EqualityRelation::get(const Variables& params, const State& /*state*/) const {
	assert(params.size() == 2);
	return params[0] == params[1];
}

void EqualityRelation::set(const Variables& /*params*/, State& /*state*/, const bool& /*value*/) const {
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

VariablesRanges EqualityRelation::getRange(const Variables& params, const State& /*state*/, const size_t constantsCount) const {
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
