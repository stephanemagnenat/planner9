#ifndef PLANNER9_HPP_
#define PLANNER9_HPP_


#include "plan.hpp"
#include "logic.hpp"
#include "domain.hpp"
#include <iostream>
#include <limits>

struct Problem;


struct Planner9 {

	struct CostFunction;
	
	Planner9(const Scope& problemScope, const CostFunction* costFunction, std::ostream* debugStream = 0);
	virtual ~Planner9() {}
	
	typedef double Cost;
	static const Cost InfiniteCost;
	
	// nodes in our search tree
	struct SearchNode {
		SearchNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state);
		friend std::ostream& operator<<(std::ostream& os, const SearchNode& node);
		
		const Plan plan;
		const TaskNetwork network; // T
		const size_t allocatedVariablesCount;
		const Cost cost;
		const CNF preconditions;
		const State state;
	};
	
	//! functions that return the cost of a node
	struct CostFunction {
		virtual Cost getCost(const SearchNode& node) const = 0;
		virtual std::string getName() const = 0;
	};
	
protected:
	void visitNode(const SearchNode* node);
	void pushNode(const Plan& plan, const TaskNetwork& network, size_t freeVariablesCount, Cost cost, const CNF& preconditions, const State& state);
	virtual void pushNode(SearchNode* node) = 0;
	virtual void success(const Plan& plan) = 0;
	
private:
	typedef std::pair<Substitution, CNF> Grounding;
	typedef std::vector<Grounding> Groundings;
	Groundings ground(const VariablesSet& variables, const CNF& preconditions, const State& state, size_t allocatedVariablesCount);
	void visitNode(const Plan& plan, const TaskNetwork& network, size_t freeVariablesCount, Cost cost, const CNF& preconditions, const State& state);

protected:
	const Scope problemScope;
	const CostFunction* costFunction;
	std::ostream*const debugStream;
};

struct SimplePlanner9: Planner9 {

	SimplePlanner9(const Scope& problemScope, const CostFunction* costFunction, std::ostream* debugStream = 0);
	SimplePlanner9(const Problem& problem, const CostFunction* costFunction, std::ostream* debugStream = 0);
	~SimplePlanner9();
	
	boost::optional<Plan> plan();
	bool plan(size_t steps);
	
	SearchNode* popNode();
	virtual void pushNode(SearchNode* node);
	virtual void success(const Plan& plan);

	typedef std::multimap<Cost, SearchNode*> SearchNodes;
	typedef std::vector<Plan> Plans;

	SearchNodes nodes;
	Plans plans;
	size_t iterationCount;
};

#endif // PLANNER9_HPP_
