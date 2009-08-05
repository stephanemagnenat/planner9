#ifndef PROBLEMS_JUG_POURING_HPP_
#define PROBLEMS_JUG_POURING_HPP_

#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"


struct MyDomain {

	Relation jug;
	Function<int> amount;
	Function<int> capacity;
	
	Action pour;
	Method pourAny;

	MyDomain();

	bool checkCapacity(int capacityJug2, int amountJug2, int amoutJug1);
};

// (defdomain basic (
MyDomain::MyDomain() :
	jug(this, "jug", 1), 
	amount(this, "amount"), 
	capacity(this, "capacity"),
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
		call(checkCapacity, capacity("jug2"), amount("jug2"), amount("jug1"))
	);
	pour.assign(amount("jug2"), amount("jug2") + amount("jug1"));
	pour.assign(amount("jug1"), 0);

	pourAny.alternative(
		"pourAny",
		jug("jug1") &&
		jug("jug2"),
		pour("jug1", "jug2")
	);
}

bool MyDomain::checkCapacity(int capacityJug2, int amountJug2, int amoutJug1)
{
	return capacityJug2 - amountJug2 >= amountJug1;
}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		add(jug("j1"));
		add(jug("j2"));
		add(jug("j3"));
		add(amount(2,"j1"));
		add(amount(10,"j2"));
		add(amount(18,"j3"));
		add(capacity(20,"j1"));
		add(capacity(20,"j2"));
		add(capacity(18,"j3"));
		goal(pourAny() >> pourAny());
	}

};

#endif // PROBLEMS_BASIC_HPP_
