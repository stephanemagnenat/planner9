#ifndef PROBLEMS_ROBOTS_PROBA2_HPP_
#define PROBLEMS_ROBOTS_PROBA2_HPP_


#include "../core/domain.hpp"
#include "../core/problem.hpp"
#include "../core/relations.hpp"

/*
  This domain/problem tests the probabilitistic action-based cost
  with contextualised-action success rates.
 */

struct MyDomain2: Domain {
	Relation isBall, isGlass;

	Action takeBall, takeGlass, dropObject, putObjectDown;
	
	Method takeObject;
	Method fetchObject;

	MyDomain2();
};

MyDomain2::MyDomain2():
	isBall("isBall", 1),
	isGlass("isGlass", 1),
	
	takeBall(this, "takeBall"),
	takeGlass(this, "takeGlass"),
	dropObject(this, "dropObject"),
	putObjectDown(this, "putObjectDown"),
	
	takeObject(this, "takeObject") ,
	fetchObject(this, "fetchObject") {
	
	takeBall.param("o");
	takeBall.pre(isBall("o"));
	
	takeGlass.param("o");
	takeGlass.pre(isGlass("o"));
	
	dropObject.param("o");
	dropObject.pre();
	
	putObjectDown.param("o");
	putObjectDown.pre();
	
	takeObject.param("o");
	takeObject.alternative(
		"take ball",
		isBall("o"),
		takeBall("o")
	);
	takeObject.alternative(
		"take glass",
		isGlass("o"),
		takeGlass("o")
	);
	
	fetchObject.param("o");
	fetchObject.alternative(
		"be carefull",
		True,
		takeObject("o") >> putObjectDown("o")
	);
	fetchObject.alternative(
		"just drop it",
		True,
		takeObject("o") >> dropObject("o")
	);
}

struct MyProblem2: MyDomain2, Problem {
	MyProblem2(const char *objectName) {
		add(isBall("ball"));
		add(isGlass("glass"));
		goal(fetchObject(objectName));
	}
};


#endif // PROBLEMS_ROBOTS_PROBA2_HPP_
