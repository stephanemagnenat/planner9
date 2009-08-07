#ifndef RANGE_HPP_
#define RANGE_HPP_

#include "variable.hpp"
#include <vector>
#include <ostream>
#include <map>
#include <cassert>

struct VariableRange:public std::vector<bool> {
	VariableRange() {}
	VariableRange(size_type n, const bool value	= false) : std::vector<bool>(n, value) { }
	void operator |=(const VariableRange& that) {
		assert(size() == that.size());
		for (size_t i = 0; i < size(); ++i)
			(*this)[i] = (*this)[i] || that[i];
	}
	void operator &=(const VariableRange& that) {
		assert(size() == that.size());
		for (size_t i = 0; i < size(); ++i)
			(*this)[i] = (*this)[i] && that[i];
	}
	void operator ~(void) {
		for (size_t i = 0; i < size(); ++i)
			(*this)[i] = ~ (*this)[i];
	}
	bool isEmpty() const {
		bool anyTrueFound = false;
		for (const_iterator it = begin(); it != end(); ++it) {
			anyTrueFound = anyTrueFound || (*it);
		}
		return anyTrueFound == false;
	}
	friend std::ostream& operator<<(std::ostream& os, const VariableRange& range) {
		os << "(";
		for (const_iterator it = range.begin(); it != range.end(); ++it) {
			os << *it;
			if (it + 1 != range.end())
				os << ",";
		}
		os << ")";
		return os;
	}
};
typedef std::map<Variable, VariableRange> VariablesRanges;

#endif // RANGE_HPP_
