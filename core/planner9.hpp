#ifndef PLANNER9_HPP_
#define PLANNER9_HPP_


#include "plan.hpp"
#include "logic.hpp"
#include "domain.hpp"
#include <iostream>


struct Problem;


struct Planner9 {

	Planner9(const Problem& problem, std::ostream* debugStream = 0);
	
	typedef int Cost;
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
	
protected:
	void visitNode(const SearchNode* node);
	void addNode(const Plan& plan, const TaskNetwork& network, size_t freeVariablesCount, Cost cost, const CNF& preconditions, const State& state);
	virtual void addNode(SearchNode* node) = 0;
	virtual void success(const Plan& plan) = 0;

private:
	void visitNode(const Plan& plan, const TaskNetwork& network, size_t freeVariablesCount, Cost cost, const CNF& preconditions, const State& state);

protected:
	const Problem& problem;
	std::ostream*const debugStream;

};


#endif // PLANNER9_HPP_
