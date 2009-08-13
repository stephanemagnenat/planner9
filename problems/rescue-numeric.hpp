#ifndef PROBLEMS_RESCUE_HPP_
#define PROBLEMS_RESCUE_HPP_


#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"
#include <boost/lambda/lambda.hpp>
namespace lambda = boost::lambda;


struct MyDomain: Domain {

	Relation robots, area;
	Function<int> extinguisher, medkit;
	EquivalentRelation isAdjacent, isConnected;
	Relation isIn;

	Action move, takeExtinguisher, takeMedkit, dropMedkit, setConnected, extinguish;
	Method moveIfNeeded, connectArea, moveWithRobots, rescue;

	MyDomain();
};

MyDomain::MyDomain():
	robots("robots", 1),
	area("area", 1),
	extinguisher("extinguisher", 1),
	medkit("extinguisher", 1),
	isAdjacent("isAdjacent"),
	isConnected("isConnected"),
	isIn("isIn", 2),
	
	move(this, "move"),
	setConnected(this, "setConnected"),
	takeExtinguisher(this, "takeExtinguisher"),
	takeMedkit(this, "takeMedkit"),
	dropMedkit(this, "dropMedkit"),
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
	
	takeExtinguisher.param("r");
	takeExtinguisher.param("a");
	takeExtinguisher.pre(
		area("a") &&
		call<bool, int>(lambda::_1 > 0, extinguisher("a"))
	);
	takeExtinguisher.assign(extinguisher("a"), call<int, int>(lambda::_1 - 1, extinguisher("a")));
	
	takeMedkit.param("r");
	takeMedkit.param("a");
	takeMedkit.pre(
		area("a") &&
		call<bool, int>(lambda::_1 > 0, medkit("a"))
	);
	takeMedkit.assign(medkit("a"), call<int, int>(lambda::_1 - 1, medkit("a")));

	dropMedkit.param("r");
	dropMedkit.param("a");
	dropMedkit.pre(
		area("a")
	);
	dropMedkit.assign(medkit("a"), call<int, int>(lambda::_1 + 1, medkit("a")));

	setConnected.param("d");
	setConnected.param("s");
	setConnected.pre();
	setConnected.add(isConnected("s", "d"));	

	extinguish.param("d");
	extinguish.param("s");
	extinguish.param("rob");
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
		area("resa") &&
		call<bool, int>(lambda::_1 > 0, extinguisher("resa")) &&
		isConnected("s", "roba") &&
		isConnected("s", "resa") &&
		isAdjacent("s", "d") &&
		!isConnected("s", "d"),
		moveIfNeeded("resa", "rob") >> takeExtinguisher("rob", "resa") >> moveIfNeeded("s", "rob") >> extinguish("d", "s", "rob")
	);
	connectArea.alternative(
		"recursion on source",
		area("d") &&
		area("s") &&
		robots("rob") &&
		area("roba") &&
		isIn("rob", "roba") &&
		area("resa") &&
		call<bool, int>(lambda::_1 > 0, extinguisher("resa")) &&
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
		area("d") &&
		area("s") &&
		area("a") &&
		robots("r") &&
		isIn("r", "a"),
		connectArea("s", "a") >> moveIfNeeded("s", "r") >> takeMedkit("r", "s") >> moveIfNeeded("d", "r") >> dropMedkit("r", "d")
	);

	rescue.param("d");
	rescue.alternative(
		"rescue",
		area("d") &&
		area("s") &&
		call<bool, int>(lambda::_1 > 0, medkit("s")),
		connectArea("d", "s") >> moveWithRobots("o", "d", "s")
	);
}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
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
		add(isIn("r0", "a0"));
		add(isIn("r1", "a1"));
		add(medkit("a0"), 1);
		add(extinguisher("a0"), 1);
		add(extinguisher("a1"), 1);
		add(extinguisher("a2"), 2);
		//goal(rescue("a5")/* >> move("o0", "a5")*/);
		goal(rescue("a5"));
		//goal(extinguish("a1", "a0", "r0", "ext0"));
		//goal(rescue("a1"));
		//goal(moveIfNeeded("a1", "r1"));
	}

};


#endif // PROBLEMS_RESCUE_HPP_
