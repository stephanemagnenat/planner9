#include "problem.hpp"
#include "tasks.hpp"


void Problem::add(const ScopedProposition& scopedAtom) {
	const Atom* originalAtom = dynamic_cast<const Atom*>(scopedAtom.proposition);
	assert(originalAtom != 0);
	Atom atom(*originalAtom);
	atom.substitute(scope.merge(scopedAtom.scope));
	state.insert(atom);
}

void Problem::goal(const ScopedTaskNetwork& goal) {
	Substitution subst(scope.merge(goal.getScope()));
	network = goal.getNetwork();
	network.substitute(subst);
}
