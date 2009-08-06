#include "atomimpl.hpp"

AtomLookup::AtomLookup(const AbstractFunction* function, const Variables& params) :
	function(function),
	params(params) {
}

AtomLookup* AtomAtomLookup::clone() const {
	return new AtomLookup(*this);
}

void AtomLookup::substitute(const Substitution& subst) {
	params.substitute(subst);
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
	Function<Return>* f(boost::polymorphic_downcast<Function<Return>*>(function));
	return f->get(params, state);
}

void AtomLookup::set(const State& oldState, State& newState, const AtomLookup& AtomLookup) const {
	// ensure that we have a function returning a bool type (i.e. a Relation)
	Function<bool>* thatRelation(boost::polymorphic_downcast<Function<bool>*>(AtomLookup.function));
	const bool val(get<bool>(oldState));
	thatRelation->set(AtomLookup.params, newState, val);
}


void AtomCall::substitute(const Substitution& subst) {
	for (size_t i = 0; i < BoostUserFunction::arity; ++i) {
		 params[i].substitute(subst);
	}
}

bool AtomCall::isCheckable(const size_t constantsCount) const {
	for (size_t i = 0; i < BoostUserFunction::arity; ++i){
		if (!params[i].isCheckable(constantsCount)) {
			return false;
		}
	}
	return true;
}

bool AtomCall::check(const State& state) const {
	return get<bool>(state);
}

void AtomCall::dump(std::ostream& os) const {
	os << "user function of type " << typeid(userFunction).name();
	for (size_t i = 0; i < BoostUserFunction::arity; ++i) {
		params[i].dump(os);
		os << "\t";
	}
}
