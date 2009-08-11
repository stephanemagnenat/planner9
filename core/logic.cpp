#include "logic.hpp"
#include "domain.hpp"
#include "relations.hpp"
#include "expressions.hpp"
#include <stdexcept>
#include <boost/mpl/assert.hpp>
#include <boost/cast.hpp>
namespace mpl = boost::mpl;

Atom::Atom(const Atom& that) :
	function(that.function),
	params(that.params) {
}

Atom::Atom(const Lookup<bool>& that) :
	function(that.function),
	params(that.params) {
}

Atom::Atom(const BoolFunction* function, const Variables& params):
	function(function),
	params(params) {
}

Atom::~Atom() {
}

Atom* Atom::clone() const {
	return new Atom(*this);
}

CNF Atom::cnf() const {
	const Literal literal(*this, false);
	const Clause clause(literal);
	return CNF(clause);
}

DNF Atom::dnf() const {
	const Literal literal(*this, false);
	const DNF::Conjunction conjunction(1, literal);
	return DNF(conjunction);
}


void Atom::substitute(const Substitution& subst) {
	params.substitute(subst);
}

void Atom::registerFunctions(Domain* domain) {
	domain->registerFunction(function);
}

void Atom::groundIfUnique(const State& state, const size_t constantsCount, Substitution& subst) const {
	function->groundIfUnique(params, state, constantsCount, subst);
}

bool Atom::isCheckable(const size_t constantsCount) const {
	for (Variables::const_iterator it = params.begin(); it != params.end(); ++it) {
	if (it->index >= constantsCount) {
			return false;
		}
	}
	return true;
}

bool Atom::check(const State& state) const {
	return function->get(params, state);
}

std::ostream& operator<<(std::ostream& os, const Atom& atom) {
	if (atom.function->name.empty() && atom.params.size() == 1)
		return os << atom.params;
	else
		return os << atom.function->name << "(" << atom.params << ")";
}



Not::Not(Proposition* proposition):
	proposition(proposition) {
}

Not::~Not() {
	delete proposition;
}

Not* Not::clone() const {
	return new Not(proposition->clone());
}

CNF Not::cnf() const {
	// use de Morgan to create the CNF out of the DNF
	const DNF dnf = proposition->dnf();
	CNF cnf;
	for(DNF::const_iterator it = dnf.begin(); it != dnf.end(); ++it) {
		CNF::Disjunction dis;
		for(DNF::Conjunction::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
			dis.push_back(Literal(jt->atom, !jt->negated));
		}
		cnf.push_back(dis);
	}
	return cnf;
}

DNF Not::dnf() const {
	// use de Morgan to create the DNF out of the CNF
	const CNF cnf = proposition->cnf();
	DNF dnf;
	for(CNF::const_iterator it = cnf.begin(); it != cnf.end(); ++it) {
		DNF::Conjunction con;
		for(CNF::Disjunction::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
			con.push_back(Literal(jt->atom, !jt->negated));
		}
		dnf.push_back(con);
	}
	return dnf;
}

void Not::substitute(const Substitution& subst) {
	proposition->substitute(subst);
}

void Not::registerFunctions(Domain* domain) {
	proposition->registerFunctions(domain);
}


Or::Or(const Propositions& propositions):
	propositions(propositions) {
}

Or::~Or() {
	for(Propositions::iterator it = propositions.begin(); it != propositions.end(); ++it) {
		delete *it;
	}
}

Or* Or::clone() const {
	Propositions propositions;
	propositions.reserve(this->propositions.size());
	for(Propositions::const_iterator it = this->propositions.begin(); it != this->propositions.end(); ++it) {
		const Proposition* proposition = *it;
		propositions.push_back(proposition->clone());
	}
	return new Or(propositions);
}

CNF Or::cnf() const {
	return dnf().cnf();
}

DNF Or::dnf() const {
	DNF result;
	for(Propositions::const_iterator it = this->propositions.begin(); it != this->propositions.end(); ++it) {
		const Proposition* proposition = *it;
		result += proposition->dnf();
	}
	return result;
}


void Or::substitute(const Substitution& subst) {
	for(Propositions::iterator it = propositions.begin(); it != propositions.end(); ++it) {
		Proposition* proposition = *it;
		proposition->substitute(subst);
	}
}

void Or::registerFunctions(Domain* domain) {
	for(Propositions::iterator it = propositions.begin(); it != propositions.end(); ++it) {
		Proposition* proposition(*it);
		proposition->registerFunctions(domain);
	}
}


And::And(const Propositions& propositions):
	propositions(propositions) {
}

And::~And() {
	for(Propositions::iterator it = propositions.begin(); it != propositions.end(); ++it) {
		delete *it;
	}
}

And* And::clone() const {
	Propositions propositions;
	propositions.reserve(this->propositions.size());
	for(Propositions::const_iterator it = this->propositions.begin(); it != this->propositions.end(); ++it) {
		const Proposition* proposition = *it;
		propositions.push_back(proposition->clone());
	}
	return new And(propositions);
}

CNF And::cnf() const {
	CNF result;
	for(Propositions::const_iterator it = propositions.begin(); it != propositions.end(); ++it) {
		const Proposition* proposition = *it;
		result += proposition->cnf();
	}
	return result;
}

DNF And::dnf() const {
	return cnf().dnf();
}

void And::substitute(const Substitution& subst) {
	for(Propositions::iterator it = propositions.begin(); it != propositions.end(); ++it) {
		Proposition* proposition = *it;
		proposition->substitute(subst);
	}
}

void And::registerFunctions(Domain* domain) {
	for(Propositions::iterator it = propositions.begin(); it != propositions.end(); ++it) {
		Proposition* proposition(*it);
		proposition->registerFunctions(domain);
	}
}


Literal::Literal(const Atom& atom, bool negated):
	atom(atom),
	negated(negated) {
}

Literal* Literal::clone() const {
	return new Literal(*this);
}

CNF Literal::cnf() const {
	const CNF::Disjunction disjunction(*this);
	return CNF(disjunction);
}

DNF Literal::dnf() const {
	const DNF::Conjunction conjunction(1, *this);
	return DNF(conjunction);
}

void Literal::substitute(const Substitution& subst) {
	atom.substitute(subst);
}

void Literal::registerFunctions(Domain* domain) {
	atom.registerFunctions(domain);
}

std::ostream& operator<<(std::ostream& os, const Literal& literal) {
	if(literal.negated)
		os << "!";
	os << literal.atom;
	return os;
}


Clause::Clause() {
}

Clause::Clause(const Literal& literal):
	std::vector<Literal>(1, literal) {
}

Clause* Clause::clone() const {
	return new Clause(*this);
}

CNF Clause::cnf() const {
	return CNF(*this);
}

DNF Clause::dnf() const {
	DNF dnf;
	for(const_iterator it = begin(); it != end(); ++it) {
		dnf.push_back(DNF::Conjunction(1, *it));
	}
	return dnf;
}

void Clause::substitute(const Substitution& subst) {
	for(iterator it = begin(); it != end(); ++it) {
		it->substitute(subst);
	}
}

void Clause::registerFunctions(Domain* domain) {
	for(iterator it = begin(); it != end(); ++it) {
		it->registerFunctions(domain);
	}
}


CNF::CNF() {
}

CNF::CNF(const Disjunction& disjunction):
	std::vector<Disjunction>(1, disjunction) {
}

CNF* CNF::clone() const {
	return new CNF(*this);
}

CNF CNF::cnf() const {
	return *this;
}

DNF CNF::dnf() const {
	DNF dnf;
	if (!empty()) {
		for(CNF::Disjunction::const_iterator jt = front().begin(); jt != front().end(); ++jt) {
			DNF::Conjunction con(1, *jt);
			dnf.push_back(con);
		}
		for(CNF::const_iterator it = begin() + 1; it != end(); ++it) {
			DNF newDnf;
			for(DNF::iterator kt = dnf.begin(); kt != dnf.end(); ++kt) {
				for(CNF::Disjunction::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
					DNF::Conjunction con(*kt);
					con.push_back(*jt);
					newDnf.push_back(con);
				}
			}
			std::swap(dnf, newDnf);
		}
	}
	return dnf;
}

/// Simplifies the CNF, returns true if it was successful, false if simpliciation lead to an unsatisfiable proposition.
/// If it returns false, cnf is untouched, otherwise it is updated with the simplified version

// TODO: continue from here

OptionalVariables CNF::simplify(const State& state, const size_t variablesBegin, const size_t variablesEnd) {
	bool wasSimplified;
	Substitution subst(Substitution::identity(variablesEnd));
	do {
		wasSimplified = false;

		// DPLL Unit propagation: check which variables can be trivially grounded
		for(iterator it = begin(); it != end(); ++it) {
			if(it->size() == 1) {
				const Literal& literal = it->front();
				if(!literal.negated) {
					const Atom& atom(literal.atom);
					atom.groundIfUnique(state, variablesBegin, subst);
				}
			}
		}

		// substitute CNF with grounded variables
		substitute(subst);

		// simplify CNF
		CNF newCnf;
		for(iterator it = begin(); it != end(); ++it) {
			Disjunction newDisjunction;
			bool disjunctionTrue = false;
			for(Disjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
				const Literal& literal(*jt);
				const Atom& atom(literal.atom);
				if (atom.isCheckable(variablesBegin) == false) {
					newDisjunction.push_back(literal);
				} else {
					wasSimplified = true;
					bool value = atom.check(state) ^ literal.negated;
					if (value == true) {
						disjunctionTrue = true;
						break;
					}
				}
			}
			if (disjunctionTrue == false) {
				if (newDisjunction.empty()) {
					// disjunction is empty, which means that all literals of the disjunction were false
					return false;
				}
				newCnf.push_back(newDisjunction);
			}
		}
		*this = newCnf;
	}
	while (wasSimplified);

	return subst;
}

void CNF::substitute(const Substitution& subst) {
	for(iterator it = begin(); it != end(); ++it) {
		for(Disjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
			jt->substitute(subst);
		}
	}
}

void CNF::registerFunctions(Domain* domain) {
	for(iterator it = begin(); it != end(); ++it) {
		for(Disjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
			jt->registerFunctions(domain);
		}
	}
}

void CNF::operator+=(const CNF& that) {
	insert(end(), that.begin(), that.end());
}

std::ostream& operator<<(std::ostream& os, const CNF& cnf) {
	for(CNF::const_iterator it = cnf.begin(); it != cnf.end(); ++it) {
		if(it != cnf.begin()) {
			//os << " && ";
			os << " ∧ ";
		}
		const CNF::Disjunction& dis = *it;
		if(dis.size() > 1) {
			os << "(";
		}
		for(CNF::Disjunction::const_iterator jt = dis.begin(); jt != dis.end(); ++jt) {
			if(jt != it->begin()) {
				//os << " || ";
				os << " ∨ ";
			}
			os << *jt;
		}
		if(dis.size() > 1) {
			os << ")";
		}
	}
	return os;
}


DNF::DNF() {
}

DNF::DNF(const Conjunction& conjunction):
	std::vector<Conjunction>(1, conjunction) {
}

DNF* DNF::clone() const {
	return new DNF(*this);
}

CNF DNF::cnf() const {
	CNF cnf;
	if (!empty()) {
		for(DNF::Conjunction::const_iterator jt = front().begin(); jt != front().end(); ++jt) {
			CNF::Disjunction dis(*jt);
			cnf.push_back(dis);
		}
		for(DNF::const_iterator it = begin() + 1; it != end(); ++it) {
			CNF newCnf;
			for(CNF::iterator kt = cnf.begin(); kt != cnf.end(); ++kt) {
				for(DNF::Conjunction::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
					CNF::Disjunction dis(*kt);
					dis.push_back(*jt);
					newCnf.push_back(dis);
				}
			}
			std::swap(cnf, newCnf);
		}
	}
	return cnf;
}

DNF DNF::dnf() const {
	return *this;
}


void DNF::substitute(const Substitution& subst) {
	for(iterator it = begin(); it != end(); ++it) {
		for(Conjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
			jt->substitute(subst);
		}
	}
}

void DNF::registerFunctions(Domain* domain) {
	for(iterator it = begin(); it != end(); ++it) {
		for(Conjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
			jt->registerFunctions(domain);
		}
	}
}

void DNF::operator+=(const DNF& that) {
	insert(end(), that.begin(), that.end());
}

std::ostream& operator<<(std::ostream& os, const DNF& dnf) {
	for(DNF::const_iterator it = dnf.begin(); it != dnf.end(); ++it) {
		if(it != dnf.begin()) {
			//os << " || ";
			os << " ∨ ";
		}
		const DNF::Conjunction& con = *it;
		if(con.size() > 1) {
			os << "(";
		}
		for(DNF::Conjunction::const_iterator jt = con.begin(); jt != con.end(); ++jt) {
			if(jt != it->begin()) {
				//os << " && ";
				os << " ∧ ";
			}
			os << *jt;
		}
		if(con.size() > 1) {
			os << ")";
		}
	}
	return os;
}

