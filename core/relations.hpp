#ifndef RELATIONS_HPP_
#define RELATIONS_HPP_

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

	virtual void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;

	std::string name;
	size_t arity;
};

template<typename CoDomain>
struct Function : AbstractFunction {
	typedef CoDomain Storage;

	struct Lookup: Expression<CoDomain> {

	};

	ScopedExpression<CoDomain> operator()(const char* first, ...) {
		Scope::Names names;
		names.push_back(first);
		va_list vargs;
		va_start(vargs, first);
		for (size_t i = 1; i < arity; ++i)
			names.push_back(va_arg(vargs, const char*));
		va_end(vargs);

		Scope scope(names);
		Variables variables = Variables::identity(arity);

		return ScopedExpression<CoDomain>(scope, new Lookup(this, variables));
	}

	virtual CoDomain get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const CoDomain& value) const;
};

template<typename UserFunction>
struct CallFunction: Function<typename boost::function<UserFunction>::result_type> {
	typedef typename boost::function<UserFunction> BoostUserFunction;
	typedef typename boost::function_types::parameter_types<UserFunction>::type UserFunctionArgs;
	typedef typename BoostUserFunction::result_type ResultType;
	typedef typename boost::mpl::transform<UserFunctionArgs, Lookup<boost::mpl::_1> >::type LookupTypes;
	typedef typename fusion::result_of::as_vector<LookupTypes> Lookups;

	struct Call: Expression<ResultType> {

	};

	BoostUserFunction userFunction;
	Lookups lookups;

	CallFunction(const BoostUserFunction& userFunction, Lookups lookups) :
		userFunction(userFunction),
		lookups(lookups) {
	}

/*
	CallFunction* clone() const {
		return new CallFunction<UserFunction>(*this);
	}

	void substitute(const Substitution& subst) {
		for (size_t i = 0; i < BoostUserFunction::arity; ++i) {
			 params[i].substitute(subst);
		}
	}

	void groundIfUnique(const State& state, const size_t constantsCount, Substitution& subst) const {
	}

	VariablesRanges getRange(const State& state, const size_t constantsCount) const {
		return VariablesRanges();
	}

	bool isCheckable(const size_t constantsCount) const {
		for (size_t i = 0; i < BoostUserFunction::arity; ++i){
			if (!params[i].isCheckable(constantsCount)) {
				return false;
			}
		}
		return true;
	}

	bool check(const State& state) const {
		return get<bool>(state);
	}

	void dump(std::ostream& os) const {
		os << "user function of type " << typeid(userFunction).name();
		for (size_t i = 0; i < BoostUserFunction::arity; ++i) {
			params[i].dump(os);
			os << "\t";
		}
	}
*/

	struct LookupArg {
		Variables::const_iterator it;
		const State& state;

		LookupArg(const Variables& params, const State& state) :
			it(params.begin()),
			state(state) {
		}

		template<typename ArgType>
		ArgType operator()(const Function<ArgType>::Lookup& lookup) {
			Variables::const_iterator begin(it), end(it + lookup.function->arity);
			ArgType arg = lookup.get(begin, end, state);
			it = end;
			return arg;
		}
	};

	ResultType get(const Variables& params, const State& state) const {
		LookupArg functor(params, state);

		// create a fusion sequence for the arguments
		UserFunctionArgs args(fusion::transform(lookups, functor));

		// invoke the function
		return fusion::invoke(userFunction, args);
	}

	void set(const Variables& params, State& state, const ResultType& value) const {
		assert(false);
	}
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
