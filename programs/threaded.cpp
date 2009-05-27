//#include "problems/basic.hpp"
//#include "problems/mini-robots.hpp"
#include "../problems/robots.hpp"
//#include "problems/rover.hpp"
#include "../threaded/planner9-threaded.hpp"


using namespace std;


int main(int argc, char* argv[]) {
/*
	Relation x("", 1);
	ScopedProposition proposition((x("a") && x("b")) || (x("c") && x("d")) || (x("e") && x("f") && x("g")));
	std::cout << Scope::setScope(proposition.scope);
	std::cout << "dnf:" << proposition.proposition->dnf() << std::endl;
	std::cout << "cnf:" << proposition.proposition->cnf() << std::endl;
*/
	std::ostream* dump(0);
	size_t threadsCount = 1;
	if (argc > 1) {
		threadsCount = atol(argv[1]);
		if (threadsCount == 0) {
			std::cerr << "Invalid number of thread" << std::endl;
			threadsCount = 1;
		}
	}
	if (argc > 2) {
		dump = &std::cout;
	}
	
	// FIXME: remove debug here
	//dump = &std::cerr;
	
	MyProblem problem;
	std::cout << Scope::setScope(problem.scope);
	std::cout << "initial state: "<< problem.state << std::endl;
	std::cout << "initial network: " << problem.network << std::endl;
	
	if (dump)
		*dump << Scope::setScope(problem.scope);
	
	ThreadedPlanner9 planner(problem, threadsCount, dump);

	boost::optional<Plan> plan = planner.plan();
	if(plan) {
		std::cout << "plan:\n" << *plan << std::endl;
	} else {
		std::cout << "no plan." << std::endl;
	}
	return 0;
}
