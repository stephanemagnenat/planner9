#ifndef LOGIC_HPP_
#define LOGIC_HPP_

#include "range.hpp"
#include "expressions.hpp"
#include "variable.hpp"

template<typename ResultType>
struct Function;
struct CNF;
struct DNF;
struct State;
struct Domain;

struct Proposition: Expression<bool> {
	virtual ~Proposition() {}

	virtual Proposition* clone() const = 0;
	virtual CNF cnf() const = 0;
	virtual DNF dnf() const = 0;
	virtual void substitute(const Substitution& subst) = 0;
	virtual void registerFunctions(Domain* domain) const = 0;

};

struct Atom: Proposition {
	typedef Function<bool> BoolFunction;
	
	Atom(const Atom& that);
	Atom(const Lookup<bool>& that);
	Atom(const BoolFunction* function, const Variables& params);
	~Atom();

	Atom* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);
	void registerFunctions(Domain* domain) const;
	
	void groundIfUnique(const State& state, const size_t constantsCount, Substitution& subst) const;
	VariablesRanges getRange(const State& state, const size_t constantsCount) const;
	bool isCheckable(const size_t constantsCount) const;
	bool check(const State& state) const;

	friend std::ostream& operator<<(std::ostream& os, const Atom& atom);

	const BoolFunction* function;
	Variables params;
};

struct Not: Proposition {

	Not(Proposition* proposition);
	~Not();

	Not* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);
	void registerFunctions(Domain* domain) const;

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
	void registerFunctions(Domain* domain) const;

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
	void registerFunctions(Domain* domain) const;

	Propositions propositions;

};

// TODO: make atom a pointer
struct Literal: Proposition {

	Literal(const Atom& atom, bool negated);

	Literal* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);
	void registerFunctions(Domain* domain) const;

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
	void registerFunctions(Domain* domain) const;

};

struct CNF: Proposition, std::vector<Clause> {

	typedef Clause Disjunction;

	CNF();
	CNF(const Disjunction& disjunction);

	CNF* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);
	void registerFunctions(Domain* domain) const;
	
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
	void registerFunctions(Domain* domain) const;

	void operator+=(const DNF& that);

	friend std::ostream& operator<<(std::ostream& os, const DNF& cnf);

};

#endif // LOGIC_HPP_
