#include "scope.hpp"
#include "variable.hpp"
#include <algorithm>
#include <cassert>
#include <boost/format.hpp>


static int xword = std::ios_base::xalloc();

Scope::SetScope::SetScope(const Scope& scope):
	scope(scope) {
}

std::ostream& operator<<(std::ostream& os, const Scope::SetScope& setScope) {
	setScope.set(os);
	return os;
}

void Scope::SetScope::set(std::ios& ios) const {
	ios.pword(xword) = &const_cast<Scope&>(scope);
}

const Scope& Scope::getScope(std::ios& ios) {
	return *static_cast<const Scope*>(ios.pword(xword));
}


Scope::Scope():
	names() {
}

Scope::Scope(const Name& name):
	names(Names(1, name)) {
}

Scope::Scope(const Names& names):
	names(names) {
}

Scope::Name Scope::getName(const Variable& variable) const {
	const Variable::Index& index = variable.index;
	if(index < getSize()) {
		return names[index];
	} else {
		return boost::str(boost::format("var%1%") % (index - getSize()));
	}
}

Variables Scope::getVariables(const Names& names) const {
	Variables result;
	result.reserve(names.size());

	Names::const_iterator begin = this->names.begin();
	Names::const_iterator end = this->names.end();

	for(Names::const_iterator it = names.begin(); it != names.end(); ++it) {
		Names::const_iterator found = std::find(begin, end, *it);
		assert(found != end);
		result.push_back(Variable(found - begin));
	}
	return result;
}

std::ostream& operator<<(std::ostream& os, const Scope& scope) {
	for(Scope::Names::const_iterator it = scope.names.begin(); it != scope.names.end(); ++it) {
		if(it != scope.names.begin())
			os << ", ";
		os << it - scope.names.begin() << ":" << *it;
	}
	return os;
}

Substitution Scope::merge(const Scope& that) {
	Substitution subst;
	subst.reserve(that.getSize());

	for(Names::const_iterator it = that.names.begin(); it != that.names.end(); ++it) {
		Names::const_iterator found = std::find(names.begin(), names.end(), *it);
		subst.push_back(Substitution::value_type(found - names.begin()));
		if(found == names.end()) {
			names.push_back(*it);
		}
	}

	return subst;
}

Scope::SetScope Scope::setScope(const Scope& scope) {
	SetScope setScope(scope);
	return setScope;
}
