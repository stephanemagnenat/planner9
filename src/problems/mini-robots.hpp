#ifndef MINI_ROBOTS_HPP_
#define MINI_ROBOTS_HPP_



#include "../domain.hpp"
#include "../problem.hpp"
#include "../relations.hpp"


struct MyDomain {

	Relation robots, object, resource, area;
	EquivalentRelation isConnectable, isConnected;
	Relation isIn;

	Action moveObject, setConnected, makeRamp;
	Method connectArea, moveWithRobots, move;

	MyDomain();

};

MyDomain::MyDomain():
	robots("robots", 1),
	object("object", 1),
	resource("resource", 1),
	area("area", 1),
	isConnectable("isConnectable"),
	isConnected("isConnected"),
	isIn("isIn", 2),
	moveObject("moveObject"),
	setConnected("setConnected"),
	makeRamp("makeRamp"),
	connectArea("connectArea"),
	moveWithRobots("moveWithRobots"),
	move("move") {

	moveObject.param("o");
	moveObject.param("d");
	moveObject.param("s");
	moveObject.param("r");
	moveObject.pre(
		object("o") &&
		area("d") &&
		area("s") &&
		robots("r") &&
		isIn("o", "s")
	);
	moveObject.del(isIn("o", "s"));
	moveObject.add(isIn("o", "d"));
	moveObject.add(isIn("r", "d"));

	setConnected.param("d");
	setConnected.param("s");
	setConnected.pre(
		area("d") &&
		area("s")
	);
	setConnected.add(isConnected("s", "d"));

	makeRamp.param("d");
	makeRamp.param("s");
	makeRamp.param("rob");
	makeRamp.param("res");
	makeRamp.pre(
		area("d") &&
		area("s") &&
		robots("rob") &&
		resource("res")
	);
	makeRamp.add(isConnected("d", "s"));

	connectArea.param("d");
	connectArea.param("s");
	connectArea.alternative( // already connected
		area("d") &&
		area("s") &&
		isConnected("d", "s")
	);
	connectArea.alternative( // ramp creation fast
		area("d") &&
		area("s") &&
		robots("rob") &&
		area("robresa") &&
		resource("res") &&
		isIn("rob", "robresa") &&
		isIn("res", "robresa") &&
		isConnectable("s", "d") &&
		!isConnected("s", "d"),
		connectArea("s", "robresa") >> makeRamp("d", "s", "rob", "res")
	);
	connectArea.alternative( // ramp creation
		area("d") &&
		area("s") &&
		robots("rob") &&
		area("roba") &&
		area("resa") &&
		resource("res") &&
		isIn("rob", "roba") &&
		isIn("res", "resa") &&
		isConnectable("s", "d") &&
		!isConnected("s", "d"),
		connectArea("s", "roba") >> connectArea("s", "resa") >> makeRamp("d", "s", "rob", "res")
	);
	connectArea.alternative( // recursion
		area("d") &&
		area("s") &&
		area("t") &&
		!equals("t", "s") &&
		!equals("t", "d"),
		connectArea("t", "s") >> connectArea("d", "t") >> setConnected("s", "d")
	);

	moveWithRobots.param("o");
	moveWithRobots.param("d");
	moveWithRobots.param("s");
	moveWithRobots.alternative(
		object("o") &&
		area("d") &&
		area("s") &&
		area("a") &&
		robots("r") &&
		isIn("r", "a"),
		connectArea("a", "s") >> moveObject("o", "d", "s", "r")
	);

	move.param("o");
	move.param("d");
	move.alternative(
		object("o") &&
		area("d") &&
		area("s") &&
		isIn("o", "s"),
		connectArea("d", "s") >> moveWithRobots("o", "d", "s")
	);

	std::cerr << moveObject << std::endl;
	std::cerr << setConnected << std::endl;
	std::cerr << makeRamp << std::endl;
	std::cerr << connectArea << std::endl;
	std::cerr << moveWithRobots << std::endl;
	std::cerr << move << std::endl;

}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		add(object("o0"));
		add(robots("r0"));
		add(resource("nut0"));
		add(area("a0"));
		add(area("a1"));
		add(area("a2"));
		add(isConnectable("a0", "a1"));
		add(isConnectable("a1", "a2"));
		add(isIn("o0", "a0"));
		add(isIn("r0", "a0"));
		add(isIn("nut0", "a0"));
		goal(move("o0", "a0"));
	}

};



#endif // MINI_ROBOTS_HPP_