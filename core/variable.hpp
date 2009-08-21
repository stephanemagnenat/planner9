#ifndef VARIABLE_HPP_
#define VARIABLE_HPP_


#include <iostream>
#include <set>
#include <utility>
#include <vector>
#include <boost/optional.hpp>


struct Variable {

	typedef size_t Index;

	explicit Variable(Index index): index(index) {}

	bool operator==(const Variable& that) const { return this->index == that.index; }
	bool operator!=(const Variable& that) const { return this->index != that.index; }
	bool operator<(const Variable& that) const { return this->index < that.index; }
	friend std::ostream& operator<<(std::ostream& os, const Variable& variable);

	Index index;

};

typedef std::set<Variable> VariablesSet;

struct Variables;
typedef Variables Substitution;

struct Variables: std::vector<Variable> {
	Variables() {
	}
	Variables(const_iterator begin, const_iterator end):
		std::vector<Variable>(begin, end) {
	}

	void substitute(const Substitution& subst);
	size_t defrag(size_t constantsCount);
	// TODO: dead code elimination
	bool containsAny(const VariablesSet& variableSet);
	bool allLessThan(const size_t upperBound) const;

	static Substitution identity(size_t size);
	
	bool operator<(const Variables& that) const;

	friend std::ostream& operator<<(std::ostream& os, const Variables& variables);

};

typedef boost::optional<Variables> OptionalVariables;


#endif // VARIABLE_HPP_
