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
	
	Atom(const BoolFunction* function, const Variables& params);
	~Atom();

	Atom* clone() const;
	CNF cnf() const;
	DNF dnf() const;
	void substitute(const Substitution& subst);
	void registerFunctions(Domain* domain) const;
	
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

struct NormalForm: Proposition {
	typedef Function<bool> BoolFunction;
	struct Literal {
		Literal();
		Literal(const BoolFunction* function, const bool negated);
		
		const BoolFunction* function;
		Variables::size_type variables;
		bool negated;
	};
	typedef std::vector<Literal> Literals;
	typedef std::vector<Literals::size_type> Junctions;

	void substitute(const Substitution& subst);
	void registerFunctions(Domain* domain) const;

	Variables variables;
	Literals literals;
	Junctions junctions;

public:
	Literals::size_type junctionSize(Junctions::const_iterator it) const;
	Variables getParams(const Literal& literal) const;
	void dump(std::ostream& os, const char* junctionSeparator, const char* literalSeparator) const;
	
protected:
	struct Junction {
		void addLiteral(const Variables& variables, const Literal& literal);
		Variables variables;
		Literals literals;
	};
	
	friend class Atom;
	void addJunction(const Junction& junction);
	void toDual(NormalForm& target) const;
	void append(const NormalForm& that);
};

struct CNF: NormalForm {
	typedef Junctions Disjunctions;
	
	virtual CNF* clone() const;
	virtual CNF cnf() const;
	virtual DNF dnf() const;
	
	OptionalVariables simplify(const State& state, const size_t variablesBegin, const size_t variablesEnd);

	void operator+=(const CNF& that);

	friend std::ostream& operator<<(std::ostream& os, const CNF& cnf);
};

struct DNF: NormalForm {
	typedef Junctions Conjunctions;

	virtual DNF* clone() const;
	virtual CNF cnf() const;
	virtual DNF dnf() const;
	
	void operator+=(const DNF& that);

	friend std::ostream& operator<<(std::ostream& os, const DNF& cnf);
};

#endif // LOGIC_HPP_
