#ifndef RELATIONS_HPP_
#define RELATIONS_HPP_


#include "logic.hpp"


struct State;


struct Relation {

	Relation(const std::string& name, size_t arity);

	ScopedProposition operator()(const char* first, ...);

	virtual bool check(const Atom& atom, const State& state) const;
	virtual void set(const Literal& literal, State& state) const;

	std::string name;
	size_t arity;

};

struct EquivalentRelation : public Relation {

	EquivalentRelation(const std::string& name);

	bool check(const Atom& atom, const State& state) const;
	void set(const Literal& literal, State& state) const;

};

struct EqualityRelation: public Relation {

	EqualityRelation();

	bool check(const Atom& atom, const State& state) const;
	void set(const Literal& literal, State& state) const;

};
extern EqualityRelation equals;


#endif // RELATIONS_HPP_
