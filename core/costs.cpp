#include "costs.hpp"

Planner9::Cost AlternativesCost::getCost(const Planner9::SearchNode& node) const
{
	return node.cost + node.network.first.size() + node.network.predecessors.size();
}

std::string AlternativesCost::getName() const
{
	return "AlternativesCost";
}


Planner9::Cost ContextualizedActionCost::getCost(const Planner9::SearchNode& node) const
{
	if (node.plan.empty())
		return 0;
	
	const double defaultCost(1);
	for (size_t i = 0; i < node.plan.size(); ++i)
	{
		size_t contextSize(0);
		
	}
		
	typedef std::vector<std::string> ContextualizedAction;
	typedef std::map<ContextualizedAction, double> SuccessRates;
	
	SuccessRates successRates;
}

std::string ContextualizedActionCost::getName() const
{
	return "ContextualizedActionCost";
}