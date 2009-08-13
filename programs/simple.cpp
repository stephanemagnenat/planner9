#include "../core/planner9.hpp"

#include "../problems/jug-pouring.hpp"
//#include "../problems/basic.hpp"
//#include "../problems/rescue.hpp"
//#include "../problems/robots.hpp"
//#include "problems/rover.hpp"

using namespace std;

int main(int argc, char* argv[]) {
	std::ostream* dump(0);
	
	if (argc > 1) {
		dump = &std::cout;
	}
	
	// FIXME: remove debug here
	//dump = &std::cerr;
	
	MyProblem problem;
	std::cout << Scope::setScope(problem.scope);
	std::cout << "initial state: "<< problem.state << std::endl;
	std::cout << "initial network: " << problem.network << std::endl;
	
	SimplePlanner9 planner(problem, dump);

	boost::optional<Plan> plan = planner.plan();
	if(plan) {
		std::cout << "plan:\n" << *plan << std::endl;
	} else {
		std::cout << "no plan." << std::endl;
	}
	return 0;
}
