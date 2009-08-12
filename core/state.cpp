#include "state.hpp"
#include "relations.hpp"

std::ostream& operator<<(std::ostream& os, const State& state) {
	bool first = true;
	for(State::Functions::const_iterator it = state.functions.begin(); it != state.functions.end(); ++it) {
		it->second->dump(os, first, it->first->name);
	}
	return os;
}
