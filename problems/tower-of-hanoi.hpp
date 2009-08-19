#ifndef TOWER_OF_HANOI_HPP_
#define TOWER_OF_HANOI_HPP_

#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"
#include <boost/lambda/lambda.hpp>
namespace lambda = boost::lambda;

struct HanoiDomain: Domain {
	Relation peg;
	Function<int> nTh;
	
	Action move;
	Action decN;
	Action incN;
	Method hanoi;

	HanoiDomain();
};

HanoiDomain::HanoiDomain():
	peg("peg", 1),
	nTh("nTh", 0),

	move(this, "move"),
	decN(this, "decN"),
	incN(this, "incN"),
	hanoi(this, "hanoi") {
	
	move.param("s");
	move.param("d");
	move.pre();
				
	decN.assign(nTh(), call<int, int>(lambda::_1 - 1, nTh()));
	decN.pre();
	
	incN.assign(nTh(), call<int, int>(lambda::_1 + 1, nTh()));
	incN.pre();
	
	hanoi.param("s");
	hanoi.param("d");
	hanoi.param("by");
	
	hanoi.alternative(
		"n==1",
		peg("s") &&
		peg("d") &&
		peg("by") &&
		call<bool, int>(lambda::_1 == 1, nTh()),
		move("s", "d")
	);
	
	hanoi.alternative(
		"n!=1",
		peg("s") &&
		peg("d") &&
		peg("by") &&
		call<bool, int>(lambda::_1 != 1, nTh()),
		decN() >> hanoi("s", "by", "d") >> move("s", "d") >> hanoi("by", "d", "s") >> incN()
	);
}

struct MyProblem: HanoiDomain, Problem {
	MyProblem() {
		add(peg("A"));
		add(peg("B"));
		add(peg("C"));
		add(nTh(), 5);
		
		goal(hanoi("A", "B", "C"));
	}
};

#endif // TOWER_OF_HANOI_HPP_
