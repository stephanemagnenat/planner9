#ifndef PLANNER9THREADED_HPP_
#define PLANNER9THREADED_HPP_


#include "../core/planner9.hpp"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <set>
#include <map>


struct ThreadedPlanner9: SimplePlanner9 {
	
	ThreadedPlanner9(const Problem& problem, size_t threadsCount, std::ostream* debugStream = 0);
	
	boost::optional<Plan> plan();

	void operator()();
	
protected:
	virtual void pushNode(SearchNode* node);
	virtual void success(const Plan& plan);

private:
	bool step();

	size_t threadsCount;
	size_t workingThreadCount;
	boost::mutex mutex;
	boost::condition condition;
};


#endif // PLANNER9THREADED_HPP_
