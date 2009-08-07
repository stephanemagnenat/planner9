#ifndef RELATIONS_HPP_
#define RELATIONS_HPP_

#include "logic.hpp"
#include "expressions.hpp"
#include "range.hpp"
#include <cassert>
#include <cstdarg>
#include <map>


struct State;
struct Domain;
struct Atom;

struct AbstractFunction {
	virtual ~AbstractFunction() {}
	
	std::string name;
	size_t arity;
	
	virtual void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;
};

template<typename CoDomain>
struct Function : AbstractFunction {
	typedef CoDomain Storage;
	
	ScopedLookup<CoDomain> operator()(const char* first, ...) {
		Scope::Names names;
		names.push_back(first);
		va_list vargs;
		va_start(vargs, first);
		for (size_t i = 1; i < arity; ++i)
			names.push_back(va_arg(vargs, const char*));
		va_end(vargs);
	
		Scope scope(names);
		Variables variables = Variables::identity(arity);
	
		return ScopedLookup<CoDomain>(scope, new AtomLookup(this, variables));
	}
	
	virtual CoDomain get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const CoDomain& value) const;
};

template<typename CoDomain>
struct SymmetricFunction : public Function<CoDomain> {

	SymmetricFunction(Domain* domain, const std::string& name);

	virtual CoDomain get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const CoDomain& value) const;
};

struct BooleanRelationStorage {
	operator bool () { return true; }
};

struct Relation: Function<bool> {
	typedef BooleanRelationStorage Storage;
	//typedef ::ScopedProposition ScopedExpression;
	
	Relation(Domain* domain, const std::string& name, size_t arity);

	ScopedProposition operator()(const char* first, ...);
	
	virtual void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;
};

struct EquivalentRelation : public Relation {
	typedef BooleanRelationStorage Storage;
	//typedef ::ScopedProposition ScopedExpression;

	EquivalentRelation(Domain* domain, const std::string& name);
	
	virtual bool get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const bool& value) const;

	virtual void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;
};

struct EqualityRelation: public Relation {
	typedef BooleanRelationStorage Storage;
	//typedef ::ScopedProposition ScopedExpression;

	EqualityRelation();

	virtual bool get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const bool& value) const;
	void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;
};
extern EqualityRelation equals;


#endif // RELATIONS_HPP_
