#ifndef PROBLEMS_BASIC_HPP_
#define PROBLEMS_BASIC_HPP_

#include "../domain.hpp"
#include "../problem.hpp"
#include "../relations.hpp"


// JSHOP2 example

// This extremely simple example shows some of the most essential
// features of SHOP2.

struct MyDomain {

	Relation have;

	Action pickup, drop;
	Method swap;

	MyDomain();

};

// (defdomain basic (
MyDomain::MyDomain() :
	have("have", 1), pickup("pickup"), drop("drop"), swap("swap") {

	// (:operator (!pickup ?a) () () ((have ?a)))
	pickup.param("a");
	pickup.add(have("a"));

	// (:operator (!drop ?a) ((have ?a)) ((have ?a)) ())
	drop.param("a");
	drop.pre(have("a"));
	drop.del(have("a"));

	/*
	 (:method (swap ?x ?y)
	 ((have ?x) (not (have ?y)))
	 ((!drop ?x) (!pickup ?y))
	 ((have ?y) (not (have ?x)))
	 ((!drop ?y) (!pickup ?x)))))
	 */
	swap.param("x");
	swap.param("y");
	swap.alternative(have("x") && !have("y"), drop("x") >> pickup("y"));
	swap.alternative(have("y") && !have("x"), drop("y") >> pickup("x"));

}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		/*
		 (defproblem problem basic
		 ((have kiwi)) ((swap banjo kiwi)))
		 */
		add(have("kiwi"));
		goal(swap("banjo", "kiwi"));
	}

};

#endif // PROBLEMS_BASIC_HPP_
