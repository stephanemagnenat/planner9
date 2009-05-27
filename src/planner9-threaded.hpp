#ifndef PLANNER9THREADED_HPP_
#define PLANNER9THREADED_HPP_


#include "planner.hpp"
#include "planner9.hpp"
#include <boost/thread.hpp>
#include <set>
#include <map>


struct ThreadedPlanner9: Planner9, Planner {
	
	ThreadedPlanner9(const Problem& problem, size_t threadsCount, std::ostream* debugStream = 0);
	~ThreadedPlanner9();
	
	boost::optional<Plan> plan();

	void operator()();
	
protected:
	void addNode(SearchNode* node);
	void success(const Plan& plan);

private:
	bool step();

	typedef std::multimap<Cost, SearchNode*> SearchNodes;
	typedef std::vector<Plan> Plans;

	SearchNodes nodes;
	Plans plans;
	size_t iterationCount;
	size_t threadsCount;
	size_t workingThreadCount;
	boost::mutex mutex;
	boost::condition condition;
};


#endif // PLANNER9THREADED_HPP_
