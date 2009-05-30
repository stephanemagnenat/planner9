#include "plan.hpp"


void Plan::substitute(const Substitution& subst) {
	for(Plan::iterator it = begin(); it != end(); ++it) {
		it->substitute(subst);
	}
}


std::ostream& operator<<(std::ostream& os, const Plan& plan) {
	for(Plan::const_iterator it = plan.begin(); it != plan.end(); ++it) {
		os << *it;
		if(it + 1 != plan.end())
			os << "\n";
	}
	return os;
}
