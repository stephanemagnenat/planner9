#ifndef DOMAIN_HPP_
#define DOMAIN_HPP_


#include "relations.hpp"
#include "tasks.hpp"
#include "state.hpp"
#include <memory>
#include <string>
#include <vector>
#include <boost/function.hpp>

typedef int Cost;

struct Domain {
	Domain();

public:
	const Head* getHead(size_t index) const;
	const Head* getHead(const std::string& name) const;
	size_t getHeadIndex(const Head* head) const;

	const Relation* getRelation(size_t index) const;
	const Relation* getRelation(const std::string& name) const;
	size_t getRelationIndex(const Relation* rel) const;

private:
	friend class Head;
	friend class Relation;
	void registerHead(const Head& head);
	void registerRelation(const Relation& rel);

private:
	typedef std::vector<const Head*> HeadsVector;
	typedef std::map<std::string, const Head*> HeadsNamesMap;
	typedef std::map<const Head*, size_t> HeadsReverseMap;
	HeadsVector headsVector;
	HeadsReverseMap headsReverseMap;
	HeadsNamesMap headsNamesMap;

	typedef std::vector<const Relation*> RelationsVector;
	typedef std::map<std::string, const Relation*> RelationsNamesMap;
	typedef std::map<const Relation*, size_t> RelationsReverseMap;
	RelationsVector relationsVector;
	RelationsNamesMap relationsNamesMap;
	RelationsReverseMap relationsReverseMap;
};

struct Head {

	Head(Domain* domain, const std::string& name);
	virtual ~Head() {}

	void param(const std::string& name);

	ScopedTaskNetwork operator()(const char* first, ...) const;

	const std::string name;
	Cost minCost;

	const Scope& getParamsScope() const { return paramsScope; }
	size_t getParamsCount() const { return paramsScope.getSize(); }

	friend std::ostream& operator<<(std::ostream& os, const Head& head);

protected:

	Scope paramsScope;

};

/// documentation test
struct Action: Head {

	Action(Domain* domain, const std::string& name);

	void pre();

	void pre(const ScopedProposition& precondition);

	void add(const ScopedProposition& atom);

	void del(const ScopedProposition& atom);

	struct AbstractEffet {
		virtual ~AbstractEffet() {}
		virtual State apply(const State& state, const Substitution subst) const = 0;
	};

	template<ValueType>
	struct Effect {
		Lookup<ValueType> left;
		const Function<ValueType>* function;
		Variables params;

		Effect(const Effect& that);
		Effect(const Lookup<ValueType>& left, const Function<ValueType>* function, const Variables& params);
		~Effect();
		void substitute(const Substitution& subst);
	};

	struct Effects:public std::vector<AbstractEffet*>  {
		State apply(const State& state, const Substitution subst) const;
		void substitute(const Substitution& subst);
	};

	const Scope& getScope() const { return scope; }
//	std::vector<std::pair<GroundInstance, Substitution> > groundings();
	const CNF& getPrecondition() const { return precondition; }
	const Effects& getEffects() const { return effects; }

private:

	void effect(const ScopedProposition& atom, bool negated);

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

struct CallFusion {
	Variables variables;
	Scope scope;

	template<typename ArgType>
	Expression<ArgType>* operator()(const ScopedExpression<ArgType>& arg) {
		Expression<ArgType>* result = arg.expression->clone();
		Variables argVars(scope.merge(arg));
		result->substitute(argVars);
		variables.insert(variables.end(), argVars.begin(), argVars.end());
		return result;
	}
};

template<typename UserFunction, typename ScopedArguments>
ScopedExpression<typename boost::function<UserFunction>::result_type>
callFusion(const boost::function<UserFunction>& userFunction, const ScopedArguments& arguments) {
	typedef CallFunction<UserFunction> Function;
	CallFusion callFusion;
	typename Function::Lookups lookups(boost::fusion::transform(arguments, callFusion));
	Function* function(new Function(userFunction, lookups));
	typename Function::Call* expression(new Function::Call(function, callFusion.variables));
	return ScopedExpression<Function::ResultType>(callFusion.scope, call);
}

template<typename ResultType>
ScopedExpression<ResultType>
call(const boost::function<ResultType()>& userFunction) {
	return callFusion(userFunction, boost::fusion::make_vector());
}

template<typename ResultType, typename Arg1Type>
ScopedExpression<ResultType>
call(const boost::function<ResultType(Arg1Type)>& userFunction,
	const ScopedExpression<Arg1Type>& arg1) {
	return callFusion(userFunction, boost::fusion::make_vector(arg1));
}

template<typename ResultType, typename Arg1Type, typename Arg2Type>
ScopedExpression<ResultType>
call(const boost::function<ResultType(Arg1Type, Arg2Type)>& userFunction,
	const ScopedExpression<Arg1Type>& arg1,
	const ScopedExpression<Arg2Type>& arg2) {
	return callFusion(userFunction, boost::fusion::make_vector(arg1, arg2));
}

#endif // DOMAIN_HPP_
