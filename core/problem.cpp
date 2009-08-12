#include "problem.hpp"
#include "tasks.hpp"

Problem::Problem() {

}

Problem::Problem(const Scope& constants) :
	scope(constants) {
}


void Problem::goal(const ScopedTaskNetwork& goal) {
	Substitution subst(scope.merge(goal.getScope()));
	network = goal.getNetwork();
	network.substitute(subst);
}
