#ifndef COSTS_HPP_
#define COSTS_HPP_

#include "planner9.hpp"

struct AlternativesCost: public Planner9::CostFunction
{
	virtual Planner9::Cost getPathCost(const Planner9::SearchNodeData& node, const Planner9::Cost pathPlusAlternativeCost) const;
	virtual Planner9::Cost getHeuristicCost(const Planner9::SearchNodeData& node) const;
	virtual std::string getName() const;
};

struct ContextualizedActionCost: public Planner9::CostFunction
{
	typedef std::vector<std::string> ContextualizedAction;
	typedef std::map<ContextualizedAction, double> SuccessRates;
	typedef std::map<std::string, double> SuccessUtilites;
	
	ContextualizedActionCost();
	
	virtual Planner9::Cost getPathCost(const Planner9::SearchNodeData& node, const Planner9::Cost pathPlusAlternativeCost) const;
	virtual Planner9::Cost getHeuristicCost(const Planner9::SearchNodeData& node) const;
	virtual std::string getName() const;
	
	void setSuccessUtilitise(const SuccessUtilites& utilities);
	void setSuccessRates(const SuccessRates& rates, double defaultRate);
	void setDefaultSuccessRate(double defaultRate);
	
protected:
	double maxSuccessRate;
	double defaultRate;
	SuccessUtilites successUtilities; //!< utilities for every action, must be 0 < u(a) <= 1
	SuccessRates successRates; //! success rates for every contextualised action, must be 0 <= r(ca) < 1
};

std::ostream& operator<<(std::ostream& os, const ContextualizedActionCost::ContextualizedAction& action);
std::ostream& operator<<(std::ostream& os, const ContextualizedActionCost::SuccessRates& rates);
std::ostream& operator<<(std::ostream& os, const ContextualizedActionCost::SuccessUtilites& utilities);

#endif // COSTS_HPP_
