#ifndef PROBLEMS_JUG_POURING_HPP_
#define PROBLEMS_JUG_POURING_HPP_

#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"


struct MyDomain: Domain {

	Relation jug;
	Function<int> amount;
	Function<int> capacity;
	
	Action pour;
	Method pourAny;

	MyDomain();

	static bool checkCapacity(int capacityJug2, int amountJug2, int amoutJug1);
};

// (defdomain basic (
MyDomain::MyDomain() :
	jug("jug", 1), 
	amount("amount", 1), 
	capacity("capacity", 1),
	pour(this, "pour"),
	pourAny(this, "pourAny") {

	/*
	(:action pour
		:parameters (?jug1 ?jug2 - jug)
		:precondition (>= (- (capacity ?jug2) (amount ?jug2)) (amount ?jug1))
		:effect (and (assign (amount ?jug1) 0)
		(increase (amount ?jug2) (amount ?jug1)))
	*/
	pour.param("jug1");
	pour.param("jug2");
	pour.pre(
		jug("jug1") &&
		jug("jug2") &&
		!equals("jug1", "jug2") &&
		call<bool, int, int, int>(checkCapacity, capacity("jug2"), amount("jug2"), amount("jug1"))
	);
	pour.assign(amount("jug2"), call<int, int, int>(std::plus<int>(), amount("jug2"), amount("jug1")));
	pour.assign(amount("jug1"), 0);

	pourAny.alternative(
		"pourAny",
		jug("jug1") &&
		jug("jug2"),
		pour("jug1", "jug2")
	);
}

bool MyDomain::checkCapacity(int capacityJug2, int amountJug2, int amountJug1)
{
	return amountJug1 > 0 && capacityJug2 - amountJug2 >= amountJug1;
}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		add(jug("j1"));
		add(jug("j2"));
		add(jug("j3"));
		add(amount("j1"), 2);
		add(amount("j2"), 10);
		add(amount("j3"), 18);
		add(capacity("j1"), 2);
		add(capacity("j2"), 20);
		add(capacity("j3"), 30);
		goal(pourAny() >> pourAny());
	}

};

#endif // PROBLEMS_BASIC_HPP_
