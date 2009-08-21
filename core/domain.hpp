#ifndef DOMAIN_HPP_
#define DOMAIN_HPP_


#include "relations.hpp"
#include "tasks.hpp"
#include "state.hpp"
#include "expressions.hpp"
#include <memory>
#include <string>
#include <vector>
#include <boost/function.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/lambda/lambda.hpp>

typedef int Cost;

struct Domain {
	Domain();

public:
	const Head* getHead(size_t index) const;
	const Head* getHead(const std::string& name) const;
	size_t getHeadIndex(const Head* head) const;

	const AbstractFunction* getRelation(size_t index) const;
	const AbstractFunction* getRelation(const std::string& name) const;
	size_t getRelationIndex(const AbstractFunction* rel) const;

private:
	friend class Head;
	friend class Action;
	friend class Atom;
	friend class NormalForm;
	void registerHead(const Head& head);
	void registerFunction(const AbstractFunction* function);

private:
	typedef std::vector<const Head*> HeadsVector;
	typedef std::map<std::string, const Head*> HeadsNamesMap;
	typedef std::map<const Head*, size_t> HeadsReverseMap;
	HeadsVector headsVector;
	HeadsReverseMap headsReverseMap;
	HeadsNamesMap headsNamesMap;

	typedef std::vector<const AbstractFunction*> RelationsVector;
	typedef std::map<std::string, const AbstractFunction*> RelationsNamesMap;
	typedef std::map<const AbstractFunction*, size_t> RelationsReverseMap;
	RelationsVector relationsVector;
	RelationsNamesMap relationsNamesMap;
	RelationsReverseMap relationsReverseMap;
};

struct CallFusion {
	Scope& scope;
	Variables& variables;
	
	CallFusion(Scope& scope, Variables& variables):
		scope(scope), variables(variables) {
	}

	template<typename ArgType>
	Lookup<ArgType> operator()(const ScopedLookup<ArgType>& arg) const {
		Lookup<ArgType> lookup(arg.lookup);
		Substitution subst(scope.merge(arg.scope));
		lookup.substitute(subst);
		variables.insert(variables.end(), lookup.params.begin(), lookup.params.end());
		return lookup;
	}

	template<typename T>
	struct result {
	};
	
	template<typename ArgType>
	struct result<CallFusion(const ScopedLookup<ArgType>&)> {
		typedef Lookup<ArgType> type;
	};
};

template<typename UserFunction, typename ScopedArguments>
ScopedLookup<typename boost::function<UserFunction>::result_type>
callFusion(const boost::function<UserFunction>& userFunction, const ScopedArguments& arguments) {
	typedef CallFunction<UserFunction> Function;
	typedef typename Function::ResultType FunctionResult;
	typedef ScopedLookup<FunctionResult> ReturnScopeLookup;
	typedef Lookup<FunctionResult> Lookup;
	
	//BOOST_MPL_ASSERT(( boost::is_same<typename Function::Lookups, ScopedArguments>));
	
	Scope scope;
	Variables variables;
	CallFusion callFusion(scope, variables);
	typename Function::Lookups lookups(fusion::transform(arguments, callFusion));
	const Function* function(new Function(userFunction, lookups));
	return ReturnScopeLookup(scope, Lookup(function, variables));
}

template<typename ResultType>
ScopedLookup<ResultType>
call(const boost::function<ResultType()>& userFunction) {
	return callFusion(userFunction, fusion::make_vector());
}

template<typename ResultType, typename Arg1Type>
ScopedLookup<ResultType>
call(const boost::function<ResultType(Arg1Type)>& userFunction,
	const ScopedLookup<Arg1Type>& arg1) {
	return callFusion(userFunction, fusion::make_vector(arg1));
}

template<typename ResultType, typename Arg1Type, typename Arg2Type>
ScopedLookup<ResultType>
call(const boost::function<ResultType(Arg1Type, Arg2Type)>& userFunction,
	const ScopedLookup<Arg1Type>& arg1,
	const ScopedLookup<Arg2Type>& arg2) {
	return callFusion(userFunction, fusion::make_vector(arg1, arg2));
}

template<typename ResultType, typename Arg1Type, typename Arg2Type, typename Arg3Type>
ScopedLookup<ResultType>
call(const boost::function<ResultType(Arg1Type, Arg2Type, Arg3Type)>& userFunction,
	const ScopedLookup<Arg1Type>& arg1,
	const ScopedLookup<Arg2Type>& arg2,
	const ScopedLookup<Arg3Type>& arg3) {
	return callFusion(userFunction, fusion::make_vector(arg1, arg2, arg3));
}


struct Head {

	Head(Domain* domain, const std::string& name);
	virtual ~Head() {}

	void param(const std::string& name);

	ScopedTaskNetwork operator()() const;
	ScopedTaskNetwork operator()(const char* first, ...) const;

	const std::string name;
	Cost minCost;

	const Scope& getParamsScope() const { return paramsScope; }
	size_t getParamsCount() const { return paramsScope.getSize(); }

	friend std::ostream& operator<<(std::ostream& os, const Head& head);

protected:

	Scope paramsScope;
	Domain* domain;
};

/// documentation test
struct Action: Head {

	Action(Domain* domain, const std::string& name);

	void pre();

	void pre(const ScopedProposition& precondition);

	void add(const ScopedLookup<bool>& scopedLookup);

	void del(const ScopedLookup<bool>& scopedLookup);

	struct AbstractEffect {
		virtual ~AbstractEffect() {}
		virtual AbstractEffect* clone() const = 0;
		virtual void apply(const State& state, State& newState, const Substitution subst) const = 0;
		virtual void substitute(const Substitution& subst) = 0;
		virtual void updateAffectedFunctionsAndVariables(FunctionsSet& affectedFunctions, VariablesSet& affectedVariables, const size_t constantsCount) const = 0;
	};

	template<typename ValueType>
	struct Effect: AbstractEffect {
		Lookup<ValueType> left;
		Lookup<ValueType> right;

		Effect(const Lookup<ValueType>& left, const Lookup<ValueType>& right) :
			left(left),
			right(right) {
		}
		
		virtual AbstractEffect* clone() const {
			return new Effect<ValueType>(*this);
		}
		
		virtual void apply(const State& state, State& newState, const Substitution subst) const {
			Variables leftParams(left.params);
			Variables rightParams(right.params);
			
			leftParams.substitute(subst);
			rightParams.substitute(subst);
			
			left.function->set(leftParams, newState, right.function->get(rightParams, state));
		}
		
		virtual void substitute(const Substitution& subst) {
			left.substitute(subst);
			right.substitute(subst);
		}
		
		void updateAffectedVariables(const Variables& params, VariablesSet& affectedVariables, const size_t constantsCount) const {
			for (Variables::const_iterator it = params.begin(); it != params.end(); ++it) {
				const Variable& variable(*it);
				if(variable.index >= constantsCount)
					affectedVariables.insert(variable);	
			}
		}
		
		virtual void updateAffectedFunctionsAndVariables(FunctionsSet& affectedFunctions, VariablesSet& affectedVariables, const size_t constantsCount) const {
			// for left part, get function (because it will be written) and variables (because they must be ground)
			affectedFunctions.insert(left.function);
			updateAffectedVariables(left.params, affectedVariables, constantsCount);
			// for right part, get only variables (because they must be ground)
			updateAffectedVariables(right.params, affectedVariables, constantsCount);
		}
	};

	struct Effects:public std::vector<AbstractEffect*>  {
		Effects() {}
		Effects(const Effects& that);
		~Effects();
		State apply(const State& state, const Substitution subst) const;
		void substitute(const Substitution& subst);
		void updateAffectedFunctionsAndVariables(FunctionsSet& affectedFunctions, VariablesSet& affectedVariables, const size_t constantsCount) const;
	};

	const Scope& getScope() const { return scope; }
	const CNF& getPrecondition() const { return precondition; }
	const Effects& getEffects() const { return effects; }

	template<typename ValueType>
	void assign(const ScopedLookup<ValueType>& left, const ScopedLookup<ValueType>& right) {
		scope.merge(paramsScope); // make sure we have all the params
		
		Lookup<ValueType> leftLookup(left.lookup);
		Lookup<ValueType> rightLookup(right.lookup);
		
		const Substitution leftSubst(scope.merge(left.scope));
		leftLookup.substitute(leftSubst);
		const Substitution rightSubst(scope.merge(right.scope));
		rightLookup.substitute(rightSubst);
		
		domain->registerFunction(leftLookup.function);
		domain->registerFunction(rightLookup.function);
		effects.push_back(new Effect<ValueType>(leftLookup, rightLookup));
	}
	
	template<typename ValueType>
	void assign(const ScopedLookup<ValueType>& left, const ValueType& rightValue) {
		assign(left, call<ValueType>(boost::lambda::constant(rightValue)));
	}

private:
	
	Scope scope;
	CNF precondition;
	Effects effects;
};

struct Method: Head {

	Method(Domain* domain, const std::string& name);

	void alternative(const std::string& name, const ScopedProposition& precondition, const ScopedTaskNetwork& decomposition = ScopedTaskNetwork(), Cost cost = 1);

	struct Alternative {
		std::string name;
		Scope scope;
		CNF precondition;
		TaskNetwork tasks; // TODO: free network's tasks upon delete
		Cost cost;

		Alternative(const std::string& name, const Scope& scope, const CNF& precondition, const TaskNetwork& tasks, Cost cost);
		bool operator<(const Alternative& that) const;
		friend std::ostream& operator<<(std::ostream& os, const Alternative& alternative);

	};

	typedef std::vector<Alternative> Alternatives;

	Alternatives alternatives;

	friend std::ostream& operator<<(std::ostream& os, const Method& method);

};

#endif // DOMAIN_HPP_
