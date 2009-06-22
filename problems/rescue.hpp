#ifndef PROBLEMS_RESCUE_HPP_
#define PROBLEMS_RESCUE_HPP_


#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"


struct MyDomain: Domain {

	Relation robots, object, extinguisher, area;
	EquivalentRelation isAdjacent, isConnected;
	Relation isIn;

	Action move, moveObject, setConnected, clearFire;
	Method moveIfNeeded, connectArea, moveWithRobots, rescue;

	MyDomain();

};

MyDomain::MyDomain():
	robots(this, "robots", 1),
	object(this, "object", 1),
	extinguisher(this, "extinguisher", 1),
	area(this, "area", 1),
	isAdjacent(this, "isAdjacent"),
	isConnected(this, "isConnected"),
	isIn(this, "isIn", 2),
	
	move(this, "move"),
	moveObject(this, "moveObject"),
	setConnected(this, "setConnected"),
	clearFire(this, "clearFire"),
	
	moveIfNeeded(this, "moveIfNeeded"),
	connectArea(this, "connectArea"),
	moveWithRobots(this, "moveWithRobots"),
	rescue(this, "rescue") {

	move.param("d");
	move.param("r");
	move.pre(
		area("a") &&
		isIn("r", "a")
	);
	move.del(isIn("r", "a"));
	move.add(isIn("r", "d"));

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

	clearFire.param("d");
	clearFire.param("s");
	clearFire.param("rob");
	clearFire.param("res");
	clearFire.pre(
		area("d") &&
		area("s") &&
		area("resa") &&
		robots("rob") &&
		extinguisher("res") &&
		isIn("res", "resa")
	);
	clearFire.add(isConnected("d",  "s"));
	clearFire.del(isIn("res", "resa"));

	moveIfNeeded.param("d");
	moveIfNeeded.param("r");
	moveIfNeeded.alternative(
		"already there",
		isIn("r", "d"),
		ScopedTaskNetwork(),
		0
	);
	moveIfNeeded.alternative(
		"go",
		!isIn("r", "d"),
		move("d", "r"),
		0
	);

	connectArea.param("d");
	connectArea.param("s");
	connectArea.alternative(
		"already connected",
		area("d") &&
		area("s") &&
		isConnected("d", "s")
	);
	connectArea.alternative(
		"clear fire",
		area("d") &&
		area("s") &&
		robots("rob") &&
		area("roba") &&
		isIn("rob", "roba") &&
		extinguisher("res") &&
		area("resa") &&
		isIn("res", "resa") &&
		isConnected("s", "roba") &&
		isConnected("s", "resa") &&
		isAdjacent("s", "d") &&
		!isConnected("s", "d"),
		moveIfNeeded("resa", "rob") >> moveIfNeeded("s", "rob") >> clearFire("d", "s", "rob", "res")
	);
	connectArea.alternative(
		"recursion on source",
		area("d") &&
		area("s") &&
		robots("rob") &&
		area("roba") &&
		isIn("rob", "roba") &&
		extinguisher("res") &&
		area("resa") &&
		isIn("res", "resa") &&
		isAdjacent("s", "d") &&
		!isConnected("s", "d"),
		connectArea("s", "roba") >> connectArea("s", "resa") >> connectArea("d", "s"),
		3
	);
	connectArea.alternative(
		"recursion on target",
		area("d") &&
		area("s") &&
		area("t") &&
		!equals("t", "s") &&
		!equals("t", "d") &&
		!isConnected("s", "d"),
		connectArea("t", "s") >> connectArea("d", "t") >> setConnected("s", "d"),
		3
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
		connectArea("s", "a") >> moveIfNeeded("s", "r") >> moveIfNeeded("d", "r") >> moveObject("o", "d", "s", "r")
	);

	rescue.param("d");
	rescue.alternative(
		"rescue",
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
		add(robots("r0"));
		add(robots("r1"));
		add(area("a0"));
		add(area("a1"));
		add(area("a2"));
		add(area("a3"));
		add(area("a4"));
		add(area("a5"));
		add(isAdjacent("a0", "a1"));
		add(isAdjacent("a0", "a3"));
		add(isAdjacent("a1", "a2"));
		add(isAdjacent("a1", "a3"));
		add(isAdjacent("a1", "a4"));
		add(isAdjacent("a4", "a5"));
		add(isIn("o0", "a0"));
		add(isIn("r0", "a0"));
		add(isIn("r1", "a1"));
		add(extinguisher("ext0"));
		add(extinguisher("ext1"));
		add(extinguisher("ext2"));
		add(extinguisher("ext3"));
		add(isIn("ext0", "a0"));
		add(isIn("ext1", "a1"));
		add(isIn("ext2", "a2"));
		add(isIn("ext3", "a2"));
		goal(rescue("a5")/* >> move("o0", "a5")*/);
		//goal(moveIfNeeded("a1", "r1"));
	}

};


#endif // PROBLEMS_RESCUE_HPP_
