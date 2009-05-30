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

typedef std::set<Variable> VariableSet;

struct Variables;
typedef Variables Substitution;

struct Variables: std::vector<Variable> {

	void substitute(const Substitution& subst);
	size_t defrag(size_t constantsCount);
	// TODO: dead code elimination
	bool containsAny(const VariableSet& variableSet);

	static Substitution identity(size_t size);

	friend std::ostream& operator<<(std::ostream& os, const Variables& variables);

};

typedef boost::optional<Variables> OptionalVariables;


#endif // VARIABLE_HPP_
