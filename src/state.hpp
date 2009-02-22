#ifndef STATE_HPP_
#define STATE_HPP_


#include "logic.hpp"
#include <set>


class State: public std::set<Atom> {

	friend std::ostream& operator<<(std::ostream& os, const State& state);

};


inline std::ostream& operator<<(std::ostream& os, const State& state) {
	for(State::const_iterator it = state.begin(); it != state.end(); ++it) {
		if(it != state.begin())
			os << ", ";
		os << *it;
	}
	return os;
}


#endif // STATE_HPP_
