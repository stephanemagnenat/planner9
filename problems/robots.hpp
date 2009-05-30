#ifndef PROBLEMS_ROBOTS_HPP_
#define PROBLEMS_ROBOTS_HPP_


#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"


struct MyDomain: Domain {

	Relation robots, object, resource, area;
	EquivalentRelation isConnectable, isConnected;
	Relation isIn;
	Relation recursionBlock;

	Action moveObject, setConnected, makeRamp;
	Action setRecursionBlock, clearRecursionBlock;
	Method connectArea, moveWithRobots, move;

	MyDomain();

};

MyDomain::MyDomain():
	robots(this, "robots", 1),
	object(this, "object", 1),
	resource(this, "resource", 1),
	area(this, "area", 1),
	isConnectable(this, "isConnectable"),
	isConnected(this, "isConnected"),
	isIn(this, "isIn", 2),
	recursionBlock(this, "recursionBlock", 2),
	
	moveObject(this, "moveObject"),
	setConnected(this, "setConnected"),
	makeRamp(this, "makeRamp"),
	setRecursionBlock(this, "setRecursionBlock"),
	clearRecursionBlock(this, "clearRecursionBlock"),
	
	connectArea(this, "connectArea"),
	moveWithRobots(this, "moveWithRobots"),
	move(this, "move") {

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
	makeRamp.add(isConnected("d",  "s"));
	
	setRecursionBlock.param("d");
	setRecursionBlock.param("s");
	setRecursionBlock.add(recursionBlock("d", "s"));
	
	clearRecursionBlock.param("d");
	clearRecursionBlock.param("s");
	clearRecursionBlock.del(recursionBlock("d", "s"));

	connectArea.param("d");
	connectArea.param("s");
	connectArea.alternative(
		"already connected",
		area("d") &&
		area("s") &&
		isConnected("d", "s")
	);
	connectArea.alternative(
		"ramp creation",
		area("d") &&
		area("s") &&
		robots("rob") &&
		area("roba") &&
		isIn("rob", "roba") &&
		resource("res") &&
		area("resa") &&
		isIn("res", "resa") &&
		isConnected("s", "roba") &&
		isConnected("s", "resa") &&
		isConnectable("s", "d") &&
		!isConnected("s", "d"),
		makeRamp("d", "s", "rob", "res")
	);
	connectArea.alternative(
		"recursion on source",
		area("d") &&
		area("s") &&
		robots("rob") &&
		area("roba") &&
		isIn("rob", "roba") &&
		resource("res") &&
		area("resa") &&
		isIn("res", "resa") &&
		isConnectable("s", "d") &&
		!isConnected("s", "d") /*&&
		!recursionBlock("d", "s")*/,
		/*setRecursionBlock("d", "s") >>*/ connectArea("s", "roba") >> connectArea("s", "resa") >> connectArea("d", "s") /*>> clearRecursionBlock("d", "s")*/,
		2
	);
	connectArea.alternative(
		"recursion on target",
		area("d") &&
		area("s") &&
		area("t") &&
		!equals("t", "s") &&
		!equals("t", "d") &&
		!isConnected("s", "d") /*&&
		!recursionBlock("d", "s")*/,
		/*setRecursionBlock("d", "s") >>*/ connectArea("t", "s") >> connectArea("d", "t") >> setConnected("s", "d") /*>> clearRecursionBlock("d", "s")*/,
		2
	);

	moveWithRobots.param("o");
	moveWithRobots.param("d");
	moveWithRobots.param("s");
	moveWithRobots.alternative(
		"moveWithRobots",
		object("o") &&
		area("d") &&
		area("s") &&
		area("a") &&
		robots("r") &&
		isIn("r", "a"),
		connectArea("s", "a") >> moveObject("o", "d", "s", "r")
	);

	move.param("o");
	move.param("d");
	move.alternative(
		"move",
		object("o") &&
		area("d") &&
		area("s") &&
		isIn("o", "s"),
		connectArea("d", "s") >> moveWithRobots("o", "d", "s")
	);
}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		add(object("o0"));
		add(object("o1"));
		add(robots("r0"));
		add(robots("r1"));
		add(resource("nut0"));
		add(resource("nut1"));
		add(area("a0"));
		add(area("a1"));
		add(area("a2"));
		add(area("a3"));
		add(area("a4"));
		add(area("a5"));
		add(area("a6"));
		add(isConnectable("a0", "a5"));
		add(isConnectable("a0", "a1"));
		add(isConnectable("a1", "a2"));
		add(isConnectable("a2", "a3"));
		add(isConnectable("a2", "a6"));
		add(isConnectable("a3", "a4"));
		add(isIn("o0", "a0"));
		add(isIn("o1", "a2"));
		add(isIn("r0", "a1"));
		//add(isIn("r1", "a6"));
		add(isIn("nut0", "a1"));
		//add(isIn("nut1", "a6"));
		goal(move("o0", "a4")/* >> move("o0", "a5")*/);
	}

};


#endif // PROBLEMS_ROBOTS_HPP_
