#ifndef LOGIC_HPP_
#define LOGIC_HPP_


#include "scope.hpp"
#include <memory>


struct Relation;
struct CNF;
struct DNF;

struct Proposition {

	virtual Proposition* clone() const = 0;
	virtual CNF cnf() const = 0;
	virtual DNF dnf() const = 0;
	virtual void substitute(const Scope::Indices& subst) = 0;

};

struct Atom: Proposition {

	Atom(const Relation* relation, const Scope::Indices& params);

	Atom* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Scope::Indices& subst);

	friend std::ostream& operator<<(std::ostream& os, const Atom& atom);

	const Relation* relation;
	Scope::Indices params;

};

struct Not: Proposition {

	Not(std::auto_ptr<Proposition> proposition);

	Not* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Scope::Indices& subst);

	std::auto_ptr<Proposition> proposition;

};

struct Or: Proposition {

	typedef std::vector<Proposition*> Propositions;

	Or(const Propositions& propositions = Propositions());

	Or* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Scope::Indices& subst);

	Propositions propositions;

};

struct And: Proposition {

	typedef std::vector<Proposition*> Propositions;

	And(const Propositions& propositions = Propositions());

	And* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Scope::Indices& subst);

	Propositions propositions;

};

struct Literal: Proposition {

	Literal(const Atom& atom, bool negated);

	Literal* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Scope::Indices& subst);

	friend std::ostream& operator<<(std::ostream& os, const Literal& literal);

	Atom atom;
	bool negated;

};

// a disjunction of literals
struct Clause: Proposition, std::vector<Literal> {

	Clause();
	Clause(const Literal& literal);

	Clause* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Scope::Indices& subst);

};

struct CNF: Proposition, std::vector<Clause> {

	typedef Clause Disjunction;

	CNF();
	CNF(const Disjunction& disjunction);

	CNF* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void simplify(); // TODO
	void substitute(const Scope::Indices& subst);

	void operator+=(const CNF& that);

	friend std::ostream& operator<<(std::ostream& os, const CNF& cnf);

};

struct DNF: Proposition, std::vector<std::vector<Literal> > {

	typedef std::vector<Literal> Conjunction;

	DNF();
	DNF(const Conjunction& conjunction);

	DNF* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Scope::Indices& subst);

	void operator+=(const DNF& that);

	friend std::ostream& operator<<(std::ostream& os, const DNF& cnf);

};



struct ScopedProposition {

	ScopedProposition(const Scope& scope, std::auto_ptr<const Proposition> proposition);

	ScopedProposition(const ScopedProposition& proposition);

	ScopedProposition operator!() const;

	ScopedProposition operator&&(const ScopedProposition& that) const;

	ScopedProposition operator||(const ScopedProposition& that) const;

	const Scope scope;
	const std::auto_ptr<const Proposition> proposition;

};


#endif // LOGIC_HPP_
