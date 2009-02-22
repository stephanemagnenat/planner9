#include "problem.hpp"
#include "tasks.hpp"


void Problem::add(const ScopedProposition& atom) {
	//TODO: state.push_back(atom.term.substitute(merge(atom.scope)));
}

void Problem::goal(const ScopedTaskNetwork& goal) {
	network = goal.getNetwork().substitute(merge(goal.getScope()));
}

Scope::Indices Problem::merge(const Scope& scope) {
	Scope::Substitutions substs = this->scope.merge(scope);
	for(State::iterator it = state.begin(); it != state.end(); ++it) {
		it->substitute(substs.first);
	}
	network.substitute(substs.first);
	return substs.second;
}
