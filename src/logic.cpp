#include "logic.hpp"
#include "relations.hpp"
#include <stdexcept>
#include <algorithm>

Atom::Atom(const Relation* relation, const Scope::Indices& params):
	relation(relation),
	params(params) {
}

bool Atom::operator<(const Atom& that) const {
	if (relation != that.relation)
		return relation < that.relation;
	return std::lexicographical_compare(params.begin(), params.end(), that.params.begin(), that.params.end());
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


void Atom::substitute(const Scope::Indices& subst) {
	params.substitute(subst);
}

std::ostream& operator<<(std::ostream& os, const Atom& atom) {
	if (atom.relation->name.empty() && atom.params.size() == 1)
		return os << atom.params;
	else
		return os << atom.relation->name << "(" << atom.params << ")";
}


Not::Not(std::auto_ptr<Proposition> proposition):
	proposition(proposition) {
}

Not* Not::clone() const {
	std::auto_ptr<Proposition> proposition(this->proposition->clone());
	return new Not(proposition);
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

void Not::substitute(const Scope::Indices& subst) {
	proposition->substitute(subst);
}


Or::Or(const Propositions& propositions):
	propositions(propositions) {
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


void Or::substitute(const Scope::Indices& subst) {
	for(Propositions::iterator it = propositions.begin(); it != propositions.end(); ++it) {
		Proposition* proposition = *it;
		proposition->substitute(subst);
	}
}


And::And(const Propositions& propositions):
	propositions(propositions) {
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
	for(Propositions::const_iterator it = this->propositions.begin(); it != this->propositions.end(); ++it) {
		const Proposition* proposition = *it;
		result += proposition->cnf();
	}
	return result;
}

DNF And::dnf() const {
	return cnf().dnf();
}

void And::substitute(const Scope::Indices& subst) {
	for(Propositions::iterator it = propositions.begin(); it != propositions.end(); ++it) {
		Proposition* proposition = *it;
		proposition->substitute(subst);
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

void Literal::substitute(const Scope::Indices& subst) {
	atom.substitute(subst);
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

void Clause::substitute(const Scope::Indices& subst) {
	for(iterator it = begin(); it != end(); ++it) {
		it->substitute(subst);
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

void CNF::substitute(const Scope::Indices& subst) {
	for(iterator it = begin(); it != end(); ++it) {
		for(Disjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
			jt->substitute(subst);
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


void DNF::substitute(const Scope::Indices& subst) {
	for(iterator it = begin(); it != end(); ++it) {
		for(Conjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
			jt->substitute(subst);
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


ScopedProposition::ScopedProposition(const Scope& scope, std::auto_ptr<const Proposition> proposition):
	scope(scope),
	proposition(proposition) {
}

ScopedProposition::ScopedProposition(const ScopedProposition& proposition):
	scope(proposition.scope),
	proposition(proposition.proposition->clone()) {
}

ScopedProposition ScopedProposition::operator!() const {
	std::auto_ptr<Proposition> proposition(this->proposition->clone());
	std::auto_ptr<const Proposition> negation(new Not(proposition));
	ScopedProposition result(scope, negation);
	return result;
}

ScopedProposition ScopedProposition::operator&&(const ScopedProposition& that) const {
	Scope scope(this->scope);
	Scope::Substitutions substs = scope.merge(that.scope);
	Proposition* left = this->proposition->clone();
	Proposition* right = that.proposition->clone();
	left->substitute(substs.first);
	right->substitute(substs.second);
	And::Propositions propositions;
	propositions.push_back(left);
	propositions.push_back(right);
	std::auto_ptr<const Proposition> result(new And(propositions));
	return ScopedProposition(scope, result);
}

ScopedProposition ScopedProposition::operator||(const ScopedProposition& that) const {
	Scope scope(this->scope);
	Scope::Substitutions substs = scope.merge(that.scope);
	Proposition* left = this->proposition->clone();
	Proposition* right = that.proposition->clone();
	left->substitute(substs.first);
	right->substitute(substs.second);
	Or::Propositions propositions;
	propositions.push_back(left);
	propositions.push_back(right);
	std::auto_ptr<const Proposition> result(new Or(propositions));
	return ScopedProposition(scope, result);
}
