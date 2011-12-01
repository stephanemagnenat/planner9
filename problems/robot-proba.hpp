#ifndef PROBLEMS_ROBOTS_PROBA_HPP_
#define PROBLEMS_ROBOTS_PROBA_HPP_


#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"

/*
  This domain/problem tests the basic probabilitistic action-based cost.
 */

struct MyDomain: Domain {
	Relation robot, region;
	EquivalentRelation isObstacle, isValley, isConnected;
	Relation isIn;

	Action move, clearObstacle, fillValley;
	Method connectRegions, moveRobot;

	MyDomain();
};

MyDomain::MyDomain():
	robot("robot", 1),
	region("region", 1),
	isObstacle("isObstacle"),
	isValley("isValley"),
	isConnected("isConnected"),
	isIn("isIn", 2),
	
	move(this, "move"),
	clearObstacle(this, "clearObstacle"),
	fillValley(this, "fillValley"),
	
	connectRegions(this, "connectRegions"),
	moveRobot(this, "moveRobot") {

	move.param("d");
	move.param("r");
	move.pre(
		region("d") &&
		region("s") &&
		robot("r") &&
		isIn("r", "s") &&
		isConnected("d", "s")
	);
	move.del(isIn("r", "s"));
	move.add(isIn("r", "d"));
	
	clearObstacle.param("d");
	clearObstacle.param("s");
	clearObstacle.pre(
		region("d") &&
		region("s") &&
		isObstacle("d", "s")
	);
	clearObstacle.add(isConnected("d", "s"));

	fillValley.param("d");
	fillValley.param("s");
	fillValley.pre(
		region("d") &&
		region("s") &&
		isObstacle("d", "s")
	);
	fillValley.add(isConnected("d", "s"));
	
	connectRegions.param("d");
	connectRegions.param("s");
	connectRegions.alternative(
		"clear obstacle",
		region("d") &&
		region("s") &&
		robot("r") &&
		isIn("r", "s") &&
		isObstacle("d", "s"),
		clearObstacle("d", "s")
	);
	connectRegions.alternative(
		"fill valley",
		region("d") &&
		region("s") &&
		robot("r") &&
		isIn("r", "s") &&
		isValley("d", "s"),
		fillValley("d", "s")
	);
	
	moveRobot.param("d");
	moveRobot.alternative(
		"already connected",
		region("d") &&
		region("s") &&
		robot("r") &&
		isIn("r", "s") &&
		!equals("d", "s") &&
		isConnected("d", "s"),
		move("d", "r")
	);
	moveRobot.alternative(
		"connect and move",
		region("d") &&
		region("s") &&
		robot("r") &&
		!equals("d", "s") &&
		isIn("r", "s"),
		connectRegions("d", "s"),
		2
	);
	moveRobot.alternative(
		"transitive recursion",
		region("d") &&
		region("s") &&
		region("t") &&
		robot("r") &&
		isIn("r", "s") && 
		!equals("d", "s") &&
		!equals("t", "s") &&
		!equals("t", "d") &&
		!isConnected("s", "d"),
		moveRobot("t") >> moveRobot("d"),
		3
	);
}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		add(robot("R"));
		add(region("A"));
		add(region("B"));
		add(region("C"));
		add(isConnected("A", "C"));
		add(isConnected("B", "C"));
		add(isObstacle("A", "B"));
		add(isValley("A", "B"));
		add(isIn("R", "A"));
		goal(moveRobot("B"));
	}

};


#endif // PROBLEMS_ROBOTS_PROBA_HPP_
