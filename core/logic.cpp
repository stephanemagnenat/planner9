#include "logic.hpp"
#include "domain.hpp"
#include "relations.hpp"
#include "expressions.hpp"
#include <stdexcept>
#include <boost/mpl/assert.hpp>
#include <boost/cast.hpp>
namespace mpl = boost::mpl;

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
	const NormalForm::Literal literal(function, false);
	NormalForm::Junction junction;
	junction.addLiteral(params, literal);
	CNF cnf;
	cnf.addJunction(junction);
	return cnf;
}

DNF Atom::dnf() const {
	const NormalForm::Literal literal(function, false);
	NormalForm::Junction junction;
	junction.addLiteral(params, literal);
	DNF dnf;
	dnf.addJunction(junction);
	return dnf;
}


void Atom::substitute(const Substitution& subst) {
	params.substitute(subst);
}

void Atom::registerFunctions(Domain* domain) const {
	domain->registerFunction(function);
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
	cnf.variables = dnf.variables;
	cnf.literals = dnf.literals;
	cnf.junctions = dnf.junctions;
	for(NormalForm::Literals::iterator it = cnf.literals.begin(); it != cnf.literals.end(); ++it) {
		it->negated = !it->negated;
	}
	return cnf;
}

DNF Not::dnf() const {
	// use de Morgan to create the DNF out of the CNF
	const CNF cnf = proposition->cnf();
	DNF dnf;
	dnf.variables = cnf.variables;
	dnf.literals = cnf.literals;
	dnf.junctions = cnf.junctions;
	for(NormalForm::Literals::iterator it = dnf.literals.begin(); it != dnf.literals.end(); ++it) {
		it->negated = !it->negated;
	}
	return dnf;
}

void Not::substitute(const Substitution& subst) {
	proposition->substitute(subst);
}

void Not::registerFunctions(Domain* domain) const {
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

void Or::registerFunctions(Domain* domain) const {
	for(Propositions::const_iterator it = propositions.begin(); it != propositions.end(); ++it) {
		const Proposition* proposition(*it);
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

void And::registerFunctions(Domain* domain) const {
	for(Propositions::const_iterator it = propositions.begin(); it != propositions.end(); ++it) {
		const Proposition* proposition(*it);
		proposition->registerFunctions(domain);
	}
}


NormalForm::Literal::Literal() {
}

NormalForm::Literal::Literal(const BoolFunction* function, const bool negated):
	function(function),
	variables(0),
	negated(negated) {
}


void NormalForm::substitute(const Substitution& subst) {
	variables.substitute(subst);
}

void NormalForm::registerFunctions(Domain* domain) const {
	for (Literals::const_iterator it = literals.begin(); it != literals.end(); ++it) {
		domain->registerFunction(it->function);
	}
}

NormalForm::Literals::size_type NormalForm::junctionSize(Junctions::const_iterator it) const {
	Junctions::const_iterator nextIt(it + 1);
	NormalForm::Literals::size_type nextLiterals;
	if (nextIt == junctions.end()) {
		nextLiterals = literals.size();
	} else {
		nextLiterals = *nextIt;
	}
	return nextLiterals - *it;
}

Variables NormalForm::getParams(const Literal& literal) const {
	Variables::const_iterator begin(variables.begin() + literal.variables);
	Variables::const_iterator end(begin + literal.function->arity);
	return Variables(begin, end);
}

void NormalForm::dump(std::ostream& os, const char* junctionSeparator, const char* literalSeparator) const {
	for(Junctions::const_iterator it = junctions.begin(); it != junctions.end(); ++it) {
		if(it != junctions.begin()) {
			os << " " << junctionSeparator << " ";
		}
		Literals::size_type size(junctionSize(it));
		if(size > 1) {
			os << "(";
		}
		Literals::const_iterator first(literals.begin() + *it);
		for(Literals::const_iterator jt = first; jt != first + size; ++jt) {
			if(jt != first) {
				os << " " << literalSeparator << " ";
			}
			const Literal& literal(*jt);
			if(literal.negated)
				//os << "¬";
				os << "!";
			const Variables params(getParams(literal));
			if (literal.function->name.empty() && params.size() == 1)
				os << params;
			else
				os << literal.function->name << "(" << params << ")";
		}
		if(size > 1) {
			os << ")";
		}
	}
}

void NormalForm::Junction::addLiteral(const Variables& variables, const Literal& literal) {
	literals.push_back(literal);
	literals.back().variables = this->variables.size();
	this->variables.insert(this->variables.end(), variables.begin(), variables.end());
}

void NormalForm::addJunction(const Junction& junction) {
	const Variables::size_type thisVarSize(this->variables.size());
	const Literals::size_type thisLitSize(this->literals.size());
	variables.insert(variables.end(), junction.variables.begin(), junction.variables.end());
	for (Literals::const_iterator it = junction.literals.begin(); it != junction.literals.end(); ++it) {
		Literal lit(*it);
		lit.variables += thisVarSize;
		literals.push_back(lit);
	}
	junctions.push_back(thisLitSize);
}

void NormalForm::toDual(NormalForm& target) const {
	std::vector<Junction> newJunctions;
	if (!junctions.empty()) {
		Junctions::const_iterator junctionsIt(junctions.begin());
		Literals::const_iterator literalsIt(literals.begin());
		
		// insert first junctions with 1 literal each
		for(; literalsIt != literals.begin() + junctionSize(junctionsIt); ++literalsIt) {
			const Literal& literal(*literalsIt);
			Junction junction;
			junction.addLiteral(getParams(literal), literal);
			newJunctions.push_back(junction);
		}
		
		for(++junctionsIt; junctionsIt != junctions.end(); ++junctionsIt) {
			std::vector<Junction> tempJunctions;
			for(std::vector<Junction>::const_iterator it = newJunctions.begin(); it != newJunctions.end(); ++it) {
				Literals::const_iterator endLiteral(literalsIt + junctionSize(junctionsIt));
				for(; literalsIt != endLiteral; ++literalsIt) {
					const Literal& literal(*literalsIt);
					Junction newJunction(*it);
					newJunction.addLiteral(getParams(literal), literal);
					tempJunctions.push_back(newJunction);
				}
			}
			std::swap(tempJunctions, newJunctions);
		}
	}
	for (std::vector<Junction>::const_iterator it = newJunctions.begin(); it != newJunctions.end(); ++it) {
		target.addJunction(*it);
	}
}

void NormalForm::append(const NormalForm& that) {
	const Variables::size_type thisVarSize(variables.size());
	const Literals::size_type thisLitSize(literals.size());
	variables.insert(variables.end(), that.variables.begin(), that.variables.end());
	for (Literals::const_iterator it = that.literals.begin(); it != that.literals.end(); ++it) {
		Literal lit(*it);
		lit.variables += thisVarSize;
		literals.push_back(lit);
	}
	for (Junctions::const_iterator it = that.junctions.begin(); it != that.junctions.end(); ++it) {
		junctions.push_back(*it + thisLitSize);
	}
}


CNF* CNF::clone() const {
	return new CNF(*this);
}

CNF CNF::cnf() const {
	return *this;
}

DNF CNF::dnf() const {
	DNF dnf;
	toDual(dnf);
	return dnf;
}

/// Simplifies the CNF, returns true if it was successful, false if simpliciation lead to an unsatisfiable proposition.
/// If it returns false, cnf is untouched, otherwise it is updated with the simplified version
OptionalVariables CNF::simplify(const State& state, const size_t variablesBegin, const size_t variablesEnd) {
	bool wasSimplified;
	Substitution subst(Substitution::identity(variablesEnd));
	do {
		wasSimplified = false;

		// DPLL Unit propagation: check which variables can be trivially grounded
		for(Disjunctions::iterator it = junctions.begin(); it != junctions.end(); ++it) {
			if(junctionSize(it) == 1) {
				const Literal& literal = literals[*it];
				if(!literal.negated) {
					const Variables params(getParams(literal));
					literal.function->groundIfUnique(params, state, variablesBegin, subst);
				}
			}
		}

		// substitute CNF with grounded variables
		substitute(subst);

		// simplify CNF
		CNF newCnf;
		for(Disjunctions::iterator it = junctions.begin(); it != junctions.end(); ++it) {
			bool disjunctionTrue = false;
			Junction newJunction;
			for(Literals::size_type j = *it; j < *it + junctionSize(it); ++j) {
				const Literal& literal = literals[j];
				const Variables params(getParams(literal));
				if (params.allLessThan(variablesBegin)) {
					wasSimplified = true;
					bool value = literal.function->get(params, state) ^ literal.negated;
					if (value == true) {
						disjunctionTrue = true;
						break;
					}
				} else {
					newJunction.addLiteral(params, literal);
				}
			}
			if (disjunctionTrue == false) {
				if (newJunction.literals.empty()) {
					// disjunction is empty, which means that all literals of the disjunction were false
					return false;
				}
				newCnf.addJunction(newJunction);
			}
		}
		std::swap(variables, newCnf.variables);
		std::swap(literals, newCnf.literals);
		std::swap(junctions, newCnf.junctions);
	}
	while (wasSimplified);

	return subst;
}

void CNF::operator+=(const CNF& that) {
	append(that);
}

std::ostream& operator<<(std::ostream& os, const CNF& cnf) {
	cnf.dump(os, "∧", "∨");
	return os;
}


DNF* DNF::clone() const {
	return new DNF(*this);
}

CNF DNF::cnf() const {
	CNF cnf;
	toDual(cnf);
	return cnf;
}

DNF DNF::dnf() const {
	return *this;
}


void DNF::operator+=(const DNF& that) {
	append(that);
}

std::ostream& operator<<(std::ostream& os, const DNF& dnf) {
	dnf.dump(os, "∨", "∧");
	return os;
}

