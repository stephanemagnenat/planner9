#ifndef RELATIONS_HPP_
#define RELATIONS_HPP_


#include "logic.hpp"


struct AbstractRelation {

};

struct Relation {

	Relation(const std::string& name, size_t arity);

	ScopedProposition operator()(const char* first, ...);

	std::string name;
	size_t arity;

};

struct EquivalentRelation : public Relation {

	EquivalentRelation(const std::string& name);

};

struct EqualityRelation: public Relation {

	EqualityRelation();

};
extern EqualityRelation equals;


#endif // RELATIONS_HPP_
