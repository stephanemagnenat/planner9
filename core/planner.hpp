#ifndef PLANNER_HPP_
#define PLANNER_HPP_


#include "plan.hpp"
#include <boost/optional.hpp>


struct Planner {
	
	virtual boost::optional<Plan> plan() = 0;
	
};


#endif // PLANNER_HPP_
