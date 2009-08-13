#ifndef RELATIONS_HPP_
#define RELATIONS_HPP_

#include "expressions.hpp"
#include "range.hpp"
#include "state.hpp"
#include <cassert>
#include <cstdarg>
#include <map>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/functional/invocation/invoke_function_object.hpp> 
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/cast.hpp>
#include <boost/units/detail/utility.hpp>

namespace mpl = boost::mpl;
namespace fusion = boost::fusion;
namespace function_types = boost::function_types;

struct State;
struct Domain;
struct Atom;

struct AbstractFunction {
	AbstractFunction(const std::string& name, size_t arity);
	virtual ~AbstractFunction() {}

	virtual void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;
	
	virtual State::AbstractFunctionState* createFunctionState() const = 0;
	
	std::string name;
	size_t arity;
};

typedef std::set<const AbstractFunction*> FunctionsSet;


template<typename CoDomain>
struct Function : AbstractFunction {	
	Function(const std::string& name, size_t arity):
		AbstractFunction(name, arity) {
	}

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

		return ScopedLookup<CoDomain>(scope, Lookup<CoDomain>(this, variables));
	}

	virtual CoDomain get(const Variables& params, const State& state) const {
		typedef State::FunctionState<CoDomain> FunctionState;
		typedef typename FunctionState::Values Values;
		
		assert(params.size() == arity);
		
		State::Functions::const_iterator it = state.functions.find(this);
		if (it == state.functions.end())
			return CoDomain();
	
		const FunctionState* functionState(boost::polymorphic_downcast<const FunctionState*>(it->second));
		typename Values::const_iterator jt(functionState->values.find(params));
		if (jt == functionState->values.end())
			return CoDomain();
			
		return jt->second;
	}
	
	virtual void set(const Variables& params, State& state, const CoDomain& value) const {
		assert(params.size() == arity);
	
		if (value != CoDomain()) {
			state.insert<CoDomain>(this, params, value);
		} else {
			state.erase<CoDomain>(this, params);
		}
	}
	
	virtual State::AbstractFunctionState* createFunctionState() const {
		return new State::FunctionState<CoDomain>();
	}
};

template<typename UserFunction>
struct CallFunction: Function<typename boost::function<UserFunction>::result_type> {
	typedef typename boost::function<UserFunction> BoostUserFunction;
	typedef typename function_types::parameter_types<UserFunction>::type UserFunctionArgsTypes;
	typedef typename BoostUserFunction::result_type ResultType;
	typedef typename fusion::result_of::as_vector<UserFunctionArgsTypes>::type UserFunctionArgs;
	typedef typename mpl::transform<UserFunctionArgs, Lookup<mpl::_1> >::type LookupTypes;
	typedef typename fusion::result_of::as_vector<LookupTypes>::type Lookups;

	BoostUserFunction userFunction;
	Lookups lookups;

	CallFunction(const BoostUserFunction& userFunction, Lookups lookups) :
		Function<ResultType>(boost::units::detail::demangle(typeid(UserFunction).name()), BoostUserFunction::arity),
		userFunction(userFunction),
		lookups(lookups) {
	}

	struct LookupArg {
		Variables::const_iterator& it;
		const State& state;

		LookupArg(Variables::const_iterator& it, const State& state) :
			it(it),
			state(state) {
		}

		template<typename ArgType>
		ArgType operator()(const Lookup<ArgType>& lookup) const {
			Variables::const_iterator begin(it), end(it + lookup.function->arity);
			Variables params;
			params.insert(params.begin(), begin, end);
			ArgType arg = lookup.function->get(params, state);
			it = end;
			return arg;
		}
		
		template<typename T>
		struct result {
		};
		
		template<typename ArgType>
		struct result<LookupArg(const Lookup<ArgType>&)> {
			typedef ArgType type;
		};
	};

	virtual ResultType get(const Variables& params, const State& state) const {
		Variables::const_iterator it(params.begin());
		LookupArg functor(it, state);
		
		// create a fusion sequence for the arguments
		UserFunctionArgs args(fusion::transform(lookups, functor));
		
		// invoke the function
		return fusion::invoke(userFunction, args);
	}

	virtual void set(const Variables& params, State& state, const ResultType& value) const {
		assert(false);
	}
};

struct Relation: Function<bool> {
	Relation(const std::string& name, size_t arity);

	virtual void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;
};

struct EquivalentRelation : public Relation {
	EquivalentRelation(const std::string& name);

	virtual bool get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const bool& value) const;

	virtual void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;
};

struct EqualityRelation: public Relation {
	EqualityRelation();

	virtual bool get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const bool& value) const;
	void groundIfUnique(const Variables& params, const State& state, const size_t constantsCount, Substitution& subst) const;
	VariablesRanges getRange(const Variables& params, const State& state, const size_t constantsCount) const;
};
extern EqualityRelation equals;


#endif // RELATIONS_HPP_
