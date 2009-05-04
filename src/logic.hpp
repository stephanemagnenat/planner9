#ifndef LOGIC_HPP_
#define LOGIC_HPP_


#include "scope.hpp"
#include "variable.hpp"


struct Relation;
struct CNF;
struct DNF;
struct State;

struct Proposition {
	virtual ~Proposition() {}

	virtual Proposition* clone() const = 0;
	virtual CNF cnf() const = 0;
	virtual DNF dnf() const = 0;
	virtual void substitute(const Substitution& subst) = 0;

};

struct Atom: Proposition {
	Atom(const Relation* relation, const Variables& params);

	bool operator<(const Atom& that) const;

	Atom* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);
	OptionalVariables unify(const Atom& atom, const size_t constantsCount, const Substitution& subst) const;
	bool isCheckable(const size_t constantsCount) const;

	friend std::ostream& operator<<(std::ostream& os, const Atom& atom);

	const Relation* relation;
	Variables params;

};

struct Not: Proposition {

	Not(Proposition* proposition);
	~Not();

	Not* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);

	Proposition *const proposition;

};

struct Or: Proposition {

	typedef std::vector<Proposition*> Propositions;

	Or(const Propositions& propositions = Propositions());
	~Or();

	Or* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);

	Propositions propositions;

};

struct And: Proposition {

	typedef std::vector<Proposition*> Propositions;

	And(const Propositions& propositions = Propositions());
	~And();

	And* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);

	Propositions propositions;

};

struct Literal: Proposition {

	Literal(const Atom& atom, bool negated);

	Literal* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);

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
	void substitute(const Substitution& subst);

};

struct CNF: Proposition, std::vector<Clause> {

	typedef Clause Disjunction;

	CNF();
	CNF(const Disjunction& disjunction);

	CNF* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);
	OptionalVariables simplify(const State& state, const size_t variablesBegin, const size_t variablesEnd);

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
	void substitute(const Substitution& subst);

	void operator+=(const DNF& that);

	friend std::ostream& operator<<(std::ostream& os, const DNF& cnf);

};



struct ScopedProposition {
	ScopedProposition();

	ScopedProposition(const Scope& scope, const Proposition* proposition);

	ScopedProposition(const ScopedProposition& proposition);

	~ScopedProposition();

	ScopedProposition operator!() const;

	ScopedProposition operator&&(const ScopedProposition& that) const;

	ScopedProposition operator||(const ScopedProposition& that) const;

	const Scope scope;
	const Proposition *const proposition;

};

extern ScopedProposition True;

#endif // LOGIC_HPP_
