#ifndef EXPRESSIONS_HPP_
#define EXPRESSIONS_HPP_

#include "scope.hpp"
#include <boost/function.hpp>
#include <boost/functional.hpp>

template<typename CoDomain>
struct Function;

template<typename Type>
struct Expression {
	virtual ~Expression() {}
	virtual Expression* clone() const = 0;
	virtual void substitute(const Substitution& subst) = 0;
};

// FIXME: rename to Call?
template<typename CoDomain>
struct Lookup: Expression<CoDomain> {
	const Function<CoDomain>* function;
	Variables params;
	
	Lookup(const Function<CoDomain>* function):
		function(function) {
	}
	
	Lookup(const Function<CoDomain>* function, const Variables& params):
		function(function),
		params(params) {
	}
	
	Lookup<CoDomain>* clone() const {
		return new Lookup<CoDomain>(*this);
	}
	
	void substitute(const Substitution& subst) {
		params.substitute(subst);
	}
};

template<typename CoDomain>
struct ScopedLookup {
	const Scope scope;
	const Lookup<CoDomain> lookup;

	ScopedLookup(const Scope& scope, const Lookup<CoDomain> lookup) :
		scope(scope),
		lookup(lookup) {
	}
	
	ScopedLookup(const Lookup<CoDomain> lookup) :
		lookup(lookup) {
	}
};


struct Proposition;

struct ScopedProposition {
	const Scope scope;
	const Proposition* proposition;
	
	ScopedProposition();
	ScopedProposition(const Scope& scope, const Proposition* expression);
	ScopedProposition(const ScopedLookup<bool>& lookup);
	ScopedProposition(const ScopedProposition& proposition);

	~ScopedProposition();

	ScopedProposition operator!() const;
	ScopedProposition operator&&(const ScopedProposition& that) const;
	ScopedProposition operator||(const ScopedProposition& that) const;
};

extern ScopedProposition True;


#endif // EXPRESSIONS_HPP_
