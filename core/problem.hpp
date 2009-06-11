#ifndef PROBLEM_HPP_
#define PROBLEM_HPP_

#include "logic.hpp"
#include "tasks.hpp"
#include "state.hpp"
#include <set>

struct Problem {

	void add(const ScopedProposition& atom);

	void goal(const ScopedTaskNetwork& goal);

	Scope scope;
	State state;
	TaskNetwork network;

};


#endif // PROBLEM_HPP_
