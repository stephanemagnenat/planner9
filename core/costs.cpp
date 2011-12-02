#include "costs.hpp"
#include <algorithm>

Planner9::Cost AlternativesCost::getPathCost(const Planner9::SearchNodeData& node, const Planner9::Cost pathPlusAlternativeCost) const {
	return pathPlusAlternativeCost;
}

Planner9::Cost AlternativesCost::getHeuristicCost(const Planner9::SearchNodeData& node) const {
	return node.network.first.size() + node.network.predecessors.size();
}

std::string AlternativesCost::getName() const {
	return "AlternativesCost";
}


ContextualizedActionCost::ContextualizedActionCost():
	defaultRate(0.5),
	maxSuccessRate(0.9)
{}

Planner9::Cost ContextualizedActionCost::getPathCost(const Planner9::SearchNodeData& node, const Planner9::Cost pathPlusAlternativeCost) const {
	if (node.plan.empty())
		return 0;
	
	// get max utility
	double pathCost(0);
	const double defaultUtility(1);
	//std::cout << "start computing path cost" << std::endl;
	for (size_t i = 0; i < node.plan.size(); ++i) {
		const std::string& actionName(node.plan[i].head->name);
		
		// get utility
		double utility;
		SuccessUtilites::const_iterator utilityIt(successUtilities.find(actionName));
		if (utilityIt == successUtilities.end())
			utility = defaultUtility;
		else
			utility = utilityIt->second;
		
		double rate(defaultRate);
		for (size_t j = 0; j <= i; ++j) {
			size_t len(i-j);
			ContextualizedAction key;
			for (size_t k = 0; k <= len; ++k)
				key.push_back(node.plan[i-k].head->name);
			SuccessRates::const_iterator it(successRates.find(key));
			if (it == successRates.end())
				break;
			rate = it->second;
			//std::cout << "Action " << actionName << ", rate found for " << key << std::endl;
		}
		
		//std::cout << "act i, utility " << utility << ", rate " << rate << ", delta cost " << (-log(utility) - log(rate)) << std::endl;
		pathCost += -log(utility) - log(rate);
	}
	//std::cout << "finished computing path cost " << pathCost << std::endl;
	return pathCost;
}

Planner9::Cost ContextualizedActionCost::getHeuristicCost(const Planner9::SearchNodeData& node) const {
	const double heuristicCost(-double(node.network.first.size() + node.network.predecessors.size()) * log(maxSuccessRate));
	return heuristicCost;
}

std::string ContextualizedActionCost::getName() const {
	return "ContextualizedActionCost";
}

// TODO: we could optimise by pre-combining the utility and the rate in the contextualised actions

void ContextualizedActionCost::setSuccessUtilitise(const SuccessUtilites& utilities) {
	if (utilities.empty())
		throw std::runtime_error("No utilities given");
	
	// check and compute max utility
	double maxUtility(0);
	for (SuccessUtilites::const_iterator it(utilities.begin()); it != utilities.end(); ++it) {
		const double utility(it->second);
		if (utility < 0)
			throw std::runtime_error("Utility must be larger or equal to 0");
		maxUtility = std::max(utility, maxUtility);
	}
	
	if (maxUtility == 0)
		throw std::runtime_error("All utilities are 0");
	
	// copy and renormalize utilities
	successUtilities = utilities;
	for (SuccessUtilites::iterator it(successUtilities.begin()); it != successUtilities.end(); ++it)
		it->second /= maxUtility;
}

void ContextualizedActionCost::setSuccessRates(const SuccessRates& rates, double defaultRate) {
	
	setDefaultSuccessRate(defaultRate);
	
	// check and compute max success rate
	double maxRate(defaultRate);
	for (SuccessRates::const_iterator it(rates.begin()); it != rates.end(); ++it) {
		const double rate(it->second);
		if (rate < 0)
			throw std::runtime_error("Success rate must be larger or equal to 0");
		if (rate >= 1)
			throw std::runtime_error("Success rate must be smaller than 1");
		maxRate = std::max(rate, maxRate);
	}
	
	if (maxRate == 0)
		throw std::runtime_error("All success rates are 0");
	
	// copy
	maxSuccessRate = maxRate;
	successRates = rates;
}

void ContextualizedActionCost::setDefaultSuccessRate(double defaultRate) {
	if (defaultRate < 0)
		throw std::runtime_error("Default success rate must be larger or equal to 0");
	if (defaultRate >= 1)
		throw std::runtime_error("Default success rate must be smaller than 1");
	this->defaultRate = defaultRate;
}


template<typename C>
void dumpVector(std::ostream& os, const C& container) {
	os << "[";
	for (size_t i = 0; i < container.size(); ++i) {
		os << container[i];
		if (i + 1 != container.size())
			os << ", ";
	}
	os << "]";
}

template<typename C>
void dumpMap(std::ostream& os, const C& container) {
	typedef typename C::const_iterator ConstIt;
	os << "{";
	size_t i(0);
	for (ConstIt it(container.begin()); it != container.end(); ++it, ++i) {
		os << it->first;
		os << ": ";
		os << it->second;
		if (i + 1 != container.size())
			os << ", ";
	}
	os << "}";
}

std::ostream& operator<<(std::ostream& os, const ContextualizedActionCost::ContextualizedAction& action) {
	dumpVector(os, action);
	return os;
}

std::ostream& operator<<(std::ostream& os, const ContextualizedActionCost::SuccessRates& rates) {
	dumpMap(os, rates);
	return os;
}

std::ostream& operator<<(std::ostream& os, const ContextualizedActionCost::SuccessUtilites& utilities) {
	dumpMap(os, utilities);
	return os;
}
