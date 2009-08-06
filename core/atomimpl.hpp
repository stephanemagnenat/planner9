#ifndef ATOMIMPL_HPP_
#define ATOMIMPL_HPP_

#include "relations.hpp"
#include "logic.hpp"
#include <boost/function.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/functional/invocation/invoke_function_object.hpp> 
#include <boost/function_types/parameter_types.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/cast.hpp>
namespace fusion = boost::fusion;
namespace function_types = boost::function_types;

struct AtomLookupArg {
	const AtomLookup* args;
	const State& state;
	size_t count;
	
	AtomLookupArg(const AtomLookup* args, const State& state) : args(args), state(state), count(0) {}
	
	template<typename Var>
	Var operator()(const Var& var) {
		return args[count++].get<Var>(state);
	}
};

template<typename UserFunction>
struct AtomCall: AtomImpl {
	typedef typename boost::function<UserFunction> BoostUserFunction;
	typedef typename function_types::parameter_types<UserFunction> UserFunctionArgs;
	typedef typename fusion::result_of::invoke<UserFunction, UserFunctionArgs>::type UserFunctionReturnType;
	
	BoostUserFunction userFunction;
	AtomLookup params[BoostUserFunction::arity];
	
	AtomCall(const BoostUserFunction& userFunction) : userFunction(userFunction) { }	
	AtomCall* clone() const { return new AtomCall<UserFunction>(*this); }
	void substitute(const Substitution& subst);
	bool isCheckable(const size_t constantsCount) const;
	bool check(const State& state) const;
	template<typename Return>
	Return get(const State& state) const {
		// make sure that the requested type is the same as the function returns
		BOOST_MPL_ASSERT((is_same<Return,UserFunctionReturnType>));
		
		// create a fusion sequence for the arguments
		UserFunctionArgs args;
		
		// fill this sequence
		fusion::transform(args, AtomLookupArg(params, state));
		
		// invoke the function
		return fusion::invoke(userFunction, args);
	}
	void set(const State& oldState, State& newState, const AtomLookup& AtomLookup) const {	
		const Function<UserFunctionReturnType>* thatFunction(boost::polymorphic_downcast<const Function<UserFunctionReturnType>*>(AtomLookup.function));
		const UserFunctionReturnType val(get<UserFunctionReturnType>(oldState));
		thatFunction->set(AtomLookup.params, newState, val);
	}
	void dump(std::ostream& os) const;
};

#endif
