#include "atomimpl.hpp"

AtomLookup::AtomLookup(const AbstractFunction* function, const Variables& params) :
	function(function),
	params(params) {
}

AtomLookup* AtomLookup::clone() const {
	return new AtomLookup(*this);
}

void AtomLookup::substitute(const Substitution& subst) {
	params.substitute(subst);
}

void AtomLookup::groundIfUnique(const State& state, const size_t constantsCount, Substitution& subst) const {
	const Relation* relation(boost::polymorphic_downcast<const Relation*>(function));
	relation->groundIfUnique(params, state, constantsCount, subst);
}

VariablesRanges AtomLookup::getRange(const State& state, const size_t constantsCount) const {
	return function->getRange(params, state, constantsCount);
}

void AtomLookup::dump(std::ostream& os) const {
	if (function->name.empty() && params.size() == 1)
		os << params;
	else
		os << function->name << "(" << params << ")";
}


bool AtomLookup::isCheckable(const size_t constantsCount) const {
	for (Variables::const_iterator it = params.begin(); it != params.end(); ++it) {
		if (it->index >= constantsCount) {
			return false;
		}
	}
	return true;
}

bool AtomLookup::check(const State& state) const {
	return get<bool>(state);
}

template<typename Return>
Return AtomLookup::get(const State& state) const {
	const Function<Return>* f(boost::polymorphic_downcast<const Function<Return>*>(function));
	return f->get(params, state);
}

void AtomLookup::set(const State& oldState, State& newState, const AtomLookup& AtomLookup) const {
	// ensure that we have a function returning a bool type (i.e. a Relation)
	const Function<bool>* thatRelation(boost::polymorphic_downcast<const Function<bool>*>(AtomLookup.function));
	const bool val(get<bool>(oldState));
	thatRelation->set(AtomLookup.params, newState, val);
}
