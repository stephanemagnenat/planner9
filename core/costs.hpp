#ifndef COSTS_HPP_
#define COSTS_HPP_

#include "planner9.hpp"

struct AlternativesCost: public Planner9::CostFunction
{
	virtual Planner9::Cost getCost(const Planner9::SearchNode& node) const;
	virtual std::string getName() const;
};

struct ContextualizedActionCost: public Planner9::CostFunction
{
	typedef std::vector<std::string> ContextualizedAction;
	typedef std::map<ContextualizedAction, double> SuccessRates;
	
	SuccessRates successRates;
	
	virtual Planner9::Cost getCost(const Planner9::SearchNode& node) const;
	virtual std::string getName() const;
};

#endif // COSTS_HPP_
