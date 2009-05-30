#ifndef STATE_HPP_
#define STATE_HPP_


#include "logic.hpp"
#include <set>



struct State {
	typedef std::set<Atom> AtomSet;
	typedef std::map<const Relation*, AtomSet> AtomsPerRelation;
	AtomsPerRelation atoms;
	
	bool contains(const Atom& atom) const {
		AtomsPerRelation::const_iterator it = atoms.find(atom.relation);
		if (it != atoms.end()) {
			return it->second.find(atom) != it->second.end();
		} else {
			return false;
		}	
	}
	
	void insert(const Atom& atom) {
		atoms[atom.relation].insert(atom);
	}
	
	void erase(const Atom& atom) {
		AtomsPerRelation::iterator it = atoms.find(atom.relation);
		if (it != atoms.end()) {
			it->second.erase(atom);
			if (it->second.empty())
				atoms.erase(atom.relation);
		}
	}
	
	friend std::ostream& operator<<(std::ostream& os, const State& state);
};


inline std::ostream& operator<<(std::ostream& os, const State& state) {
	bool first = true;
	for(State::AtomsPerRelation::const_iterator it = state.atoms.begin(); it != state.atoms.end(); ++it) {
		for (State::AtomSet::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
			if(first) {
				first = false;
			} else {
				os << ", ";
			}
			os << *jt;
		}
	}
	return os;
}


#endif // STATE_HPP_
