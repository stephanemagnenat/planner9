#ifndef SHOP2_HPP_
#define SHOP2_HPP_


#include "plan.hpp"
#include <boost/optional.hpp>
#include <set>
#include <map>


struct Problem;
struct State;
struct TaskNetwork;
struct CNF;
typedef int Cost;
struct TreeNode;


struct Planner9 {

	Planner9(const Problem& problem);

	boost::optional<Plan> plan();

private:

	void step();
	void visitNode(const Plan& plan, const TaskNetwork& network, size_t freeVariablesCount, Cost cost, const CNF& preconditions, const State& state);
	void addNode(const Plan& plan, const TaskNetwork& network, size_t freeVariablesCount, Cost cost, const CNF& preconditions, const State& state);
	void success(const Plan& plan);

	typedef std::multimap<Cost, TreeNode*> Nodes;
	typedef std::vector<Plan> Plans;

	const Problem& problem;
	Nodes nodes;
	Plans plans;
	size_t iterationCount;

};


#endif // SHOP2_HPP_
