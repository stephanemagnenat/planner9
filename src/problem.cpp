#include "problem.hpp"
#include "tasks.hpp"


void Problem::add(const ScopedProposition& scopedAtom) {
	const Atom* originalAtom = dynamic_cast<const Atom*>(scopedAtom.proposition.get());
	assert(originalAtom != 0);
	Atom atom(*originalAtom);
	atom.substitute(merge(scopedAtom.scope));
	state.insert(atom);
}

void Problem::goal(const ScopedTaskNetwork& goal) {
	Scope::Indices subst(merge(goal.getScope()));
	network = goal.getNetwork().clone();
	network.substitute(subst);
}

Scope::Indices Problem::merge(const Scope& scope) {
	Scope::Substitutions substs = this->scope.merge(scope);
	State newState;
	for(State::iterator it = state.begin(); it != state.end(); ++it) {
		Atom atom(*it);
		atom.substitute(substs.first);
		newState.insert(atom);
	}
	std::swap(newState, state);
	network.substitute(substs.first);
	return substs.second;
}
