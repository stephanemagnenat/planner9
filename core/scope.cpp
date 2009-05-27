#include "scope.hpp"
#include <algorithm>
#include <cassert>
#include <boost/format.hpp>


void Scope::Indices::substitute(const Indices& subst) {
	for(iterator it = begin(); it != end(); ++it) {
		assert(*it < subst.size());
		Index index = subst[*it];
		*it = index;
	}
}

/// regroup all used variables just after the constants, returns the new end of variables
Scope::Index Scope::Indices::defrag(const Index& constantsCount) {
	Index varIndex = constantsCount;
	for(iterator it = begin() + constantsCount; it != end(); ++it) {
		Index index = *it;
		if (index >= constantsCount) {
			*it = varIndex++;
		}
	}
	return varIndex;
}

bool Scope::Indices::containsAny(const IndexSet& indexSet) {
	for (const_iterator it = begin(); it != end(); ++it) {
		const Scope::Index index = *it;
		if (indexSet.find(index) != indexSet.end())
			return true;
	}
	return false;
}

std::ostream& operator<<(std::ostream& os, const Scope::Indices& indices) {
	const Scope& scope = Scope::getScope(os);
	for(Scope::Indices::const_iterator it = indices.begin(); it != indices.end(); ++it) {
		Scope::Index index = *it;
		if(it != indices.begin())
			os << ", ";
		os << scope.getName(index);
	}
	return os;
}

Scope::Indices Scope::Indices::identity(size_t size) {
	Indices indices;
	indices.reserve(size);
	for(size_t i = 0; i < size; ++i) {
		indices.push_back(Index(i));
	}
	return indices;
}


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

Scope::Name Scope::getName(Scope::Index index) const {
	if(index < getSize()) {
		return names[index];
	} else {
		return boost::str(boost::format("var%1%") % (index - getSize()));
	}
}

Scope::Names Scope::getNames(const Indices& indices) const {
	Scope::Names result;
	result.reserve(indices.size());
	for(Indices::const_iterator it = indices.begin(); it != indices.end(); ++it) {
		Scope::Index index = *it;
		result.push_back(getName(index));
	}
	return result;
}

Scope::Indices Scope::getIndices(const Names& names) const {
	Scope::Indices result;
	result.reserve(names.size());

	Names::const_iterator begin = this->names.begin();
	Names::const_iterator end = this->names.end();

	for(Names::const_iterator it = names.begin(); it != names.end(); ++it) {
		Names::const_iterator found = std::find(begin, end, *it);
		assert(found != end);
		result.push_back(found - begin);
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

Scope::Indices Scope::merge(const Names& names2) {
	Indices subst;
	subst.reserve(names2.size());

	for(Names::const_iterator it = names2.begin(); it != names2.end(); ++it) {
		Names::const_iterator found = std::find(names.begin(), names.end(), *it);
		subst.push_back(found - names.begin());
		if(found == names.end()) {
			names.push_back(*it);
		}
	}

	return subst;
}

Scope::Indices Scope::merge(const Scope& that) {
	return merge(that.names);
}

Scope::SetScope Scope::setScope(const Scope& scope) {
	SetScope setScope(scope);
	return setScope;
}
