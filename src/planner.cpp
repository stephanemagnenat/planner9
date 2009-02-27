//#include "problems/basic.hpp"
#include "problems/mini-robots.hpp"
#include "planner9.hpp"


using namespace std;


int main(int argc, char* argv[]) {
/*
	Relation x("", 1);
	ScopedProposition proposition((x("a") && x("b")) || (x("c") && x("d")) || (x("e") && x("f") && x("g")));
	std::cout << Scope::setScope(proposition.scope);
	std::cout << "dnf:" << proposition.proposition->dnf() << std::endl;
	std::cout << "cnf:" << proposition.proposition->cnf() << std::endl;
*/
	MyProblem problem;
	std::cout << Scope::setScope(problem.scope);
	std::cout << "initial state: "<< problem.state << std::endl;
	std::cout << "initial network: " << problem.network << std::endl;

	Planner9 planner(problem);

	boost::optional<Plan> plan = planner.plan();
	if(plan) {
		std::cout << "plan: " << *plan << std::endl;
	} else {
		std::cout << "no plan." << std::endl;
	}
	return 0;
}
