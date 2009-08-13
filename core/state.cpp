#include "state.hpp"
#include "relations.hpp"

State::State() {
}

State::State(const State& that) {
	for (Functions::const_iterator it = that.functions.begin(); it != that.functions.end(); ++it) {
		functions[it->first] = it->second->clone();
	}
}

State::~State() {
	for (Functions::const_iterator it = functions.begin(); it != functions.end(); ++it) {
		delete it->second;
	}
}

std::ostream& operator<<(std::ostream& os, const State& state) {
	bool first = true;
	for(State::Functions::const_iterator it = state.functions.begin(); it != state.functions.end(); ++it) {
		it->second->dump(os, first, it->first->name);
	}
	return os;
}
