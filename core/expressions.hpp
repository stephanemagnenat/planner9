#ifndef EXPRESSIONS_HPP_
#define EXPRESSIONS_HPP_

#include "scope.hpp"
#include <boost/function.hpp>
#include <boost/functional.hpp>

class Proposition;
class Atom;

template <typename Return>
struct ScopedAtomCall {
	ScopedAtomCall(const Scope& scope, const Atom* atom):
		scope(scope),
		atom(atom) {
	}
	
	~ScopedAtomCall() {
		delete atom;
	}
	
	const Scope scope;
	const Atom *const atom;
};

struct ScopedProposition {
	ScopedProposition();
	
	ScopedProposition(const ScopedAtomCall<bool>& atom);

	ScopedProposition(const Scope& scope, const Proposition* proposition);

	ScopedProposition(const ScopedProposition& proposition);

	~ScopedProposition();

	ScopedProposition operator!() const;

	ScopedProposition operator&&(const ScopedProposition& that) const;

	ScopedProposition operator||(const ScopedProposition& that) const;

	const Scope scope;
	const Proposition *const proposition;
};

extern ScopedProposition True;

#endif // EXPRESSIONS_HPP_
