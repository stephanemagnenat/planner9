#include "../core/planner9.hpp"
#include "../core/costs.hpp"

#include "../problems/robot-proba.hpp"
#include "../problems/robot-proba2.hpp"

using namespace std;

struct SuccessEstimatorData
{
	const double epsilon;
	const double lambda;
	double a, b;
	double last_t;
	
	SuccessEstimatorData(double r0, double epsilon, double lambda);
	SuccessEstimatorData(double epsilon, double lambda);
	void update(double t, double r);
	double getP() const;
};

SuccessEstimatorData::SuccessEstimatorData(double r0, double epsilon, double lambda):
	epsilon(epsilon),
	lambda(lambda),
	a(r0),
	b(1 + epsilon),
	last_t(0) {
}

SuccessEstimatorData::SuccessEstimatorData(double epsilon, double lambda):
	epsilon(epsilon),
	lambda(lambda),
	a(1),
	b(2),
	last_t(0) {
}

void SuccessEstimatorData::update(double t, double r) {
	const double f = exp(-lambda * (t - last_t));
	a = f * a + r;
	b = f * b + 1 + epsilon;
	last_t = t;
}

double SuccessEstimatorData::getP() const {
	return a / b;
}

double uniformRand() {
    return double(rand()) / double(RAND_MAX);
}

bool draw(double p) {
	return p >= uniformRand();
}

void testLearning(std::ostream* dump) {
	
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
	ContextualizedActionCost::ContextualizedAction aDropBall, aDropGlass, aPutDown;
	aDropBall.push_back("dropObject"); aDropBall.push_back("takeBall");
	aDropGlass.push_back("dropObject"); aDropGlass.push_back("takeGlass");;
	aPutDown.push_back("putObjectDown");
	
	const double epsilon(0.01);
	const double lambda(1 / 10);
	SuccessEstimatorData estDropBall(epsilon, lambda);
	SuccessEstimatorData estDropGlass(epsilon, lambda);
	SuccessEstimatorData estPutDown(epsilon, lambda);
	SuccessEstimatorData estOther(epsilon, lambda);
	
	const double realDropBallRate(0.9);
	const double realDropGlassRate(0.1);
	const double realPutDownRate(0.8);
	const double realOtherRate(0.9);
	
	const char* objectsToFetch[] = { "glass", "ball" };
	for (int t = 0; t < 100; ++t) {
		// object name
		const int objId(t % 2);
		const char *objectName(objectsToFetch[objId]);
		std::cout << "Time " << t << " fetching object " << objectName << std::endl;
		
		// setting rates
		rates[aDropBall] = estDropBall.getP();
		rates[aDropGlass] = estDropGlass.getP();
		rates[aPutDown] = estPutDown.getP();
		const double defaultRate = estOther.getP();
		contextualizedActionCost.setSuccessRates(rates, defaultRate);
		std::cout << "rates: " << rates << ", default: " << defaultRate << std::endl;
		
		// building problem and get plan
		MyProblem2 problem(objectName);
		std::cout << Scope::setScope(problem.scope);
		SimplePlanner9 planner(problem, &contextualizedActionCost, dump);
		boost::optional<Plan> plan = planner.plan();
		if(plan) {
			std::cout << "plan:\n" << *plan << std::endl;
		} else {
			std::cout << "no plan." << std::endl;
		}
		std::cout << std::endl;
		
		// simulate execution
		assert(plan->size() == 2);
		// step one is always other
		const bool stepOneR(draw(realOtherRate));
		estOther.update(t, stepOneR ? 1 : 0);
		if (stepOneR) {
			const std::string& task0((*plan)[0].head->name);
			const std::string& task1((*plan)[1].head->name);
			// step 2
			if (task1 == "putObjectDown") {
				estPutDown.update(t, draw(realPutDownRate) ? 1 : 0);
			} else {
				assert(task1 == "dropObject");
				if (task0 == "takeBall") {
					estDropBall.update(t, draw(realDropBallRate) ? 1 : 0);
				} else if (task0 == "takeGlass") {
					estDropGlass.update(t, draw(realDropGlassRate) ? 1 : 0);
				} else {
					assert(false);
				}
			}
		}
	}
}

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
	contextualizedActionCost.setSuccessRates(rates, 0.9);
	std::cout << "rates: " << rates << ", default: " << 0.9 << std::endl;
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
	contextualizedActionCost.setDefaultSuccessRate(0.9);
	
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
	//testContextualizedAction(dump);
	testLearning(dump);
	
	return 0;
}
