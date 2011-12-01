#include "../core/planner9.hpp"
#include "../core/costs.hpp"

#include "../problems/robot-proba.hpp"
#include "../problems/robot-proba2.hpp"

using namespace std;

void testContextualizedAction(std::ostream* dump) {
	// setup action costs
	ContextualizedActionCost contextualizedActionCost;
	ContextualizedActionCost::SuccessUtilites utilities;
	utilities["takeBall"] = 1;
	utilities["takeGlass"] = 1;
	utilities["dropObject"] = 5;
	utilities["putObjectDown"] = 1;
	contextualizedActionCost.setSuccessUtilitise(utilities);
	std::cout << "utilities: " << utilities << std::endl;
	
	// setup success rates
	ContextualizedActionCost::SuccessRates rates;
	ContextualizedActionCost::ContextualizedAction a;
	a.clear(); a.push_back("dropObject"); a.push_back("takeBall"); rates[a] = 0.9;
	a.clear(); a.push_back("dropObject"); a.push_back("takeGlass"); rates[a] = 0.1;
	a.clear(); a.push_back("putObjectDown");rates[a] = 0.8;
	contextualizedActionCost.setSuccessRates(rates);
	std::cout << "rates: " << rates << std::endl;
	std::cout << std::endl;
	
	const char* objectsToFetch[] = { "glass", "ball" };
	for (int i = 0; i < 2; ++i)
	{
		const char *objectName(objectsToFetch[i]);
		std::cout << "Fetching object " << objectName << std::endl;
		MyProblem2 problem(objectName);
		std::cout << Scope::setScope(problem.scope);
		std::cout << "initial state: "<< problem.state << std::endl;
		std::cout << "initial network: " << problem.network << std::endl;
		
		SimplePlanner9 planner(problem, &contextualizedActionCost, dump);
		boost::optional<Plan> plan = planner.plan();
		if(plan) {
			std::cout << "plan:\n" << *plan << std::endl;
		} else {
			std::cout << "no plan." << std::endl;
		}
		std::cout << std::endl;
	}
}

void testSimpleProba(std::ostream* dump) {
	MyProblem problem;
	std::cout << Scope::setScope(problem.scope);
	std::cout << "initial state: "<< problem.state << std::endl;
	std::cout << "initial network: " << problem.network << std::endl;
	
	// setup action costs
	ContextualizedActionCost contextualizedActionCost;
	ContextualizedActionCost::SuccessUtilites utilities;
	utilities["move"] = 1;
	utilities["clearObstacle"] = 0.95;
	utilities["fillValley"] = 0.93;
	contextualizedActionCost.setSuccessUtilitise(utilities);
	
	std::cout << "utilities: " << utilities << std::endl;
	
	SimplePlanner9 planner(problem, &contextualizedActionCost, dump);
	boost::optional<Plan> plan = planner.plan();
	if(plan) {
		std::cout << "plan:\n" << *plan << std::endl;
	} else {
		std::cout << "no plan." << std::endl;
	}
}

int main(int argc, char* argv[]) {
	std::ostream* dump(0);
	
	if (argc > 1) {
		dump = &std::cout;
	}
	
	//testSimpleProba(dump);
	testContextualizedAction(dump);
	
	return 0;
}
