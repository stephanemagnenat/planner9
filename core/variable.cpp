#include "variable.hpp"
#include "scope.hpp"


std::ostream& operator<<(std::ostream& os, const Variable& variable) {
	const Scope& scope = Scope::getScope(os);
	os << scope.getName(variable);
	return os;
}


void Variables::substitute(const Substitution& subst) {
	for(iterator it = begin(); it != end(); ++it) {
		Variable& variable = *it;
		assert(it->index < subst.size());
		const Variable& newVariable = subst[variable.index];
		variable = newVariable;
	}
}

/// regroup all used variables just after the constants, returns the new end of variables
size_t Variables::defrag(size_t constantsCount) {
	size_t index = constantsCount;
	for(iterator it = begin() + constantsCount; it != end(); ++it) {
		Variable& variable = *it;
		if (variable.index >= constantsCount) {
			variable = Variable(index);
			++index;
		}
	}
	return index;
}

bool Variables::containsAny(const VariableSet& variables) {
	for (const_iterator it = begin(); it != end(); ++it) {
		const Variable variable = *it;
		if (variables.find(variable) != variables.end())
			return true;
	}
	return false;
}

std::ostream& operator<<(std::ostream& os, const Variables& variables) {
	for(Variables::const_iterator it = variables.begin(); it != variables.end(); ++it) {
		Variable variable = *it;
		if(it != variables.begin())
			os << ", ";
		os << variable;
	}
	return os;
}

Substitution Substitution::identity(size_t size) {
	Substitution subst;
	subst.reserve(size);
	for(size_t i = 0; i < size; ++i) {
		subst.push_back(Variable(i));
	}
	return subst;
}
