#ifndef STATE_HPP_
#define STATE_HPP_


#include "logic.hpp"
#include <algorithm>
#include <set>



struct State {
	struct AbstractFunctionState {
		virtual void dump(std::ostream& os, bool& first) = 0;
	};
	
	template<typename Function>
	struct FunctionState: AbstractFunctionState {
		const Function* structure;
		typedef std::map<Variables, typename Function::Storage> Values;
		Values values;
		
		void dump(std::ostream& os, bool& first) {
			for (typename Values::const_iterator it = values.begin(); it != values.end(); ++it) {
				if(first) {
					first = false;
				} else {
					os << ", ";
				}
				os << structure->name << "(" << it->first << ") : " << it->second;
			}
		}
	};
	
	typedef std::map<const AbstractFunction*, AbstractFunctionState*> Functions;
	Functions functions;
	
	friend std::ostream& operator<<(std::ostream& os, const State& state);
};


inline std::ostream& operator<<(std::ostream& os, const State& state) {
	bool first = true;
	for(State::Functions::const_iterator it = state.functions.begin(); it != state.functions.end(); ++it) {
		it->second->dump(os, first);
	}
	return os;
}


#endif // STATE_HPP_
