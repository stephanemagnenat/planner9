#ifndef PROBLEMS_BASIC_HPP_
#define PROBLEMS_BASIC_HPP_

#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"


// JSHOP2 example

// This extremely simple example shows some of the most essential
// features of SHOP2.

struct MyDomain: Domain {

	Relation have;
	Relation provide;

	Action pickup, drop;
	Method swap;
	Method trocFor;

	MyDomain();

};

// (defdomain basic (
MyDomain::MyDomain() :
	have("have", 1), 
	provide("provide", 2), 
	pickup(this, "pickup"), 
	drop(this, "drop"), 
	swap(this, "swap"), 
	trocFor(this, "trocFor") {

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
	swap.alternative("swap ltr", have("x") && !have("y"), drop("x") >> pickup("y"));
	swap.alternative("swap rtl", have("y") && !have("x"), drop("y") >> pickup("x"));

	trocFor.param("x");
	trocFor.alternative("trocFor", have("y") && !have("x") && provide("p", "x"), swap("x", "y"));
}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		/*
		 (defproblem problem basic
		 ((have kiwi)) ((swap banjo kiwi)))
		 */
		add(have("kiwi"));
		add(have("lasagnes"));
		add(provide("toto", "banjo"));
		goal(trocFor("banjo"));
	}

};

#endif // PROBLEMS_BASIC_HPP_
