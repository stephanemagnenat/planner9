#include "plan.hpp"


std::ostream& operator<<(std::ostream& os, const Plan& plan) {
	for(Plan::const_iterator it = plan.begin(); it != plan.end(); ++it) {
		os << *it;
		if(it + 1 != plan.end())
			os << ", ";
	}
	return os;
}
