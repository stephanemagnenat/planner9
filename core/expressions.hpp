#ifndef EXPRESSIONS_HPP_
#define EXPRESSIONS_HPP_

#include "scope.hpp"
#include <boost/function.hpp>
#include <boost/functional.hpp>

template<typename Type>
struct Expression {
	virtual ~Expression() {}
	virtual Expression* clone() const = 0;
	virtual void substitute(const Substitution& subst) = 0;
	/* TODO: check what is usefull
	virtual void groundIfUnique(const State& state, const size_t constantsCount, Substitution& subst) const = 0;
	virtual VariablesRanges getRange(const State& state, const size_t constantsCount) const;
	virtual bool isCheckable(const size_t constantsCount) const = 0;
	virtual bool check(const State& state) const = 0;
	virtual void set(const State& oldState, State& newState, const AtomLookup& lookup) const = 0;
	virtual void dump(std::ostream& os) const = 0;
	*/
};

template<typename Type>
struct ScopedExpression {
	const Scope scope;
	const Expression<Type>* expression;

	ScopedExpression(const Scope& scope, const Expression<Type>* expression) :
		scope(scope),
		expression(expression) {
	}
};

struct Proposition;

struct ScopedProposition: ScopedExpression<bool> {
	ScopedProposition(const Scope& scope, const Proposition* expression);

	~ScopedProposition();

	ScopedProposition operator!() const;

	ScopedProposition operator&&(const ScopedProposition& that) const;

	ScopedProposition operator||(const ScopedProposition& that) const;

	const Proposition* proposition();
};

extern ScopedProposition True;

#endif // EXPRESSIONS_HPP_
