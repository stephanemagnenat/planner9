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
	std::sort(this->names.begin(), this->names.end());
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
		Names::const_iterator found = std::lower_bound(begin, end, *it);
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

Substitutions Scope::merge(const Scope& that) {
	const Names& names2 = that.names;

	Substitution subst, subst2;
	Names result;

	Names::const_iterator begin, end;
	Names::const_iterator begin2, end2;

	begin = names.begin();
	end = names.end();
	begin2 = names2.begin();
	end2 = names2.end();

	for(Names::const_iterator it = begin, it2 = begin2; (it != end) || (it2 != end2);) {
		int cmp;
		if (it2 == end2) {
			cmp = -1;
		} else if (it == end) {
			cmp = +1;
		} else {
			cmp = it->compare(*it2);
		}

		Variable variable(result.size());
		if (cmp <= 0) {
			result.push_back(*it);
		} else {
			result.push_back(*it2);
		}

		if (cmp <= 0) {
			subst.push_back(variable);
			++it;
		}
		if (cmp >= 0) {
			subst2.push_back(variable);
			++it2;
		}
	}

	std::swap(names, result);

	return make_pair(subst, subst2);
}

Scope::SetScope Scope::setScope(const Scope& scope) {
	SetScope setScope(scope);
	return setScope;
}
