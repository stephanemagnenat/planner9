#ifndef PROBLEM_HPP_
#define PROBLEM_HPP_

#include "logic.hpp"
#include "tasks.hpp"
#include "state.hpp"
#include "relations.hpp"
#include <set>
#include <boost/cast.hpp>

struct Problem {
	Problem();
	Problem(const Scope& constants);

	template<typename ValueType>
	void add(const ScopedLookup<ValueType>& scopedLookup, const ValueType& value) {
		typedef Lookup<ValueType> Lookup;
		
		Lookup lookup(scopedLookup.lookup);
		lookup.substitute(scope.merge(scopedLookup.scope));
		state.insert<ValueType>(lookup.function, lookup.params, value);
	}
	
	void add(const ScopedLookup<bool>& scopedLookup) {
		add<bool>(scopedLookup, true);
	}

	void goal(const ScopedTaskNetwork& goal);

	Scope scope;
	State state;
	TaskNetwork network;

};


#endif // PROBLEM_HPP_
