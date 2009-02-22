#ifndef PLAN_HPP_
#define PLAN_HPP_


#include "tasks.hpp"
#include <ostream>


struct Plan: std::vector<Task> {

	friend std::ostream& operator<<(std::ostream& os, const Plan& plan);

};


#endif // PLAN_HPP_
