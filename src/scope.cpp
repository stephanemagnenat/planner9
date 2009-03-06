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
	std::sort(this->names.begin(), this->names.end());
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
		Names::const_iterator found = std::lower_bound(begin, end, *it);
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

Scope::Substitutions Scope::merge(const Names& names2) {
	Indices indices, indices2;
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

		Scope::Index index = result.size();
		if (cmp <= 0) {
			result.push_back(*it);
		} else {
			result.push_back(*it2);
		}

		if (cmp <= 0) {
			indices.push_back(index);
			++it;
		}
		if (cmp >= 0) {
			indices2.push_back(index);
			++it2;
		}
	}

	std::swap(names, result);

	return make_pair(indices, indices2);
}

Scope::Substitutions Scope::merge(const Scope& that) {
	return merge(that.names);
}

Scope::SetScope Scope::setScope(const Scope& scope) {
	SetScope setScope(scope);
	return setScope;
}
