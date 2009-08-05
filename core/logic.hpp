#ifndef LOGIC_HPP_
#define LOGIC_HPP_


#include "scope.hpp"
#include "variable.hpp"
#include <boost/function.hpp>


struct AbstractFunction;
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
	struct Predicate;
	
	Atom(const Atom& that);
	Atom(const Predicate* predicate);
	Atom(const Relation* relation, const Variables& params);
	~Atom();
	
	//Atom(const Function* relation, const Variables& params);

	Atom* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);

	friend std::ostream& operator<<(std::ostream& os, const Atom& atom);
	
	struct Lookup;
	
	struct Predicate {
		virtual ~Predicate() {}
		virtual Predicate* clone() const = 0;
		virtual void substitute(const Substitution& subst) = 0;
		virtual bool isCheckable(const size_t constantsCount) const = 0;
		virtual bool check(const State& state) const = 0;
		virtual void set(const State& oldState, State& newState, const Lookup& lookup) const = 0;
		virtual void dump(std::ostream& os) const = 0;
	};
	
	struct Lookup: Predicate {
		Lookup(const AbstractFunction* function, const Variables& params);
		Lookup* clone() const;
		void substitute(const Substitution& subst);
		bool isCheckable(const size_t constantsCount) const;
		bool check(const State& state) const;
		void set(const State& oldState, State& newState, const Lookup& lookup) const ;
		void dump(std::ostream& os) const;
		
		const AbstractFunction* function;
		Variables params;
	};
	
	template<typename UserFunction>
	struct Call: Predicate {
		Call(const boost::function<UserFunction>& userFunction);
		Call* clone() const;
		void substitute(const Substitution& subst);
		bool isCheckable(const size_t constantsCount) const;
		bool check(const State& state) const;
		void set(const State& oldState, State& newState, const Lookup& lookup) const ;
		void dump(std::ostream& os) const;
		
		typedef std::vector<Lookup> Args;
		boost::function<UserFunction> userFunction;
		Args params;
	};
	
	Predicate* predicate;
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

// TODO: make atom a pointer
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

template<typename Return>
struct ScopedLookup {
	ScopedLookup(const Scope& scope, const Atom::Lookup& lookup);
	
	const Scope scope;
	const Atom::Lookup lookup;
};

extern ScopedProposition True;

ScopedProposition call(const boost::function<bool()>& userFunction);
template<typename T1>
ScopedProposition call(const boost::function<bool(T1)>& userFunction, const ScopedLookup<T1>& arg1);
template<typename T1, typename T2>
ScopedProposition call(const boost::function<bool(T1,T2)>& userFunction, const ScopedLookup<T1>& arg1, const ScopedLookup<T2>& arg2);

#endif // LOGIC_HPP_
