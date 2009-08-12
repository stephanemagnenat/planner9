#ifndef PROBLEMS_RESCUE_HPP_
#define PROBLEMS_RESCUE_HPP_


#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"


struct MyDomain: Domain {

	Relation robots, object, extinguisher, area;
	EquivalentRelation isAdjacent, isConnected;
	Relation isIn;

	Action move, drop, setConnected, take, extinguish;
	Method moveIfNeeded, connectArea, moveWithRobots, rescue;

	MyDomain();

};

MyDomain::MyDomain():
	robots("robots", 1),
	object("object", 1),
	extinguisher("extinguisher", 1),
	area("area", 1),
	isAdjacent("isAdjacent"),
	isConnected("isConnected"),
	isIn("isIn", 2),
	
	move(this, "move"),
	drop(this, "drop"),
	setConnected(this, "setConnected"),
	take(this, "take"),
	extinguish(this, "extinguish"),
	
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

	drop.param("r");
	drop.param("o");
	drop.pre(
		area("a") &&
		isIn("r", "a")
	);
	drop.add(isIn("o", "a"));

	setConnected.param("d");
	setConnected.param("s");
	setConnected.pre();
	setConnected.add(isConnected("s", "d"));
	
	take.param("r");
	take.param("o");
	take.pre(
		area("a") &&
		isIn("o", "a")
	);
	take.del(isIn("o", "a"));

	extinguish.param("d");
	extinguish.param("s");
	extinguish.param("rob");
	extinguish.param("res");
	extinguish.pre();
	extinguish.add(isConnected("d",  "s"));

	moveIfNeeded.param("d");
	moveIfNeeded.param("r");
	moveIfNeeded.alternative(
		"already there",
		area("d") &&
		robots("r") &&
		isIn("r", "d"),
		ScopedTaskNetwork(),
		0
	);
	moveIfNeeded.alternative(
		"go",
		area("d") &&
		robots("r") &&
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
		moveIfNeeded("resa", "rob") >> take("rob", "res") >> moveIfNeeded("s", "rob") >> extinguish("d", "s", "rob", "res")
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
		connectArea("s", "a") >> moveIfNeeded("s", "r") >> take("r", "o") >> moveIfNeeded("d", "r") >> drop("r", "o")
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
		//goal(extinguish("a1", "a0", "r0", "ext0"));
		//goal(rescue("a1"));
		//goal(moveIfNeeded("a1", "r1"));
	}

};


#endif // PROBLEMS_RESCUE_HPP_
