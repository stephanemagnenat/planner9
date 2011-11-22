#include "planner9-threaded.hpp"
#include "../core/plan.hpp"
#include "../core/problem.hpp"

ThreadedPlanner9::ThreadedPlanner9(const Problem& problem, size_t threadsCount, const UserCost* userCost, std::ostream* debugStream):
	SimplePlanner9(problem, userCost, debugStream),
	threadsCount(threadsCount),
	workingThreadCount(0) {
}

// HTN: procedure SHOP2(s, T, D)
boost::optional<Plan> ThreadedPlanner9::plan() {
	boost::mutex::scoped_lock lock(mutex);

	boost::thread_group threads;
	for (size_t i = 0; i < threadsCount; ++i) {
		threads.create_thread(boost::ref(*this));
		workingThreadCount++;
	}
	
	lock.unlock();
	threads.join_all();
	
	std::cout << "Terminated after " << iterationCount << " iterations" << std::endl;

	if(plans.empty())
		return false;
	else
		return plans.front();
}

bool ThreadedPlanner9::step() {
	boost::mutex::scoped_lock lock(mutex);
	workingThreadCount--;
	
	do {
		if (!plans.empty())
			return false;
		if (nodes.empty()) {
			if(workingThreadCount == 0)
				return false;
			else
				condition.wait(lock);
		}
	} while (nodes.empty() || !plans.empty());
	
	SearchNode* node = popNode();
	
	workingThreadCount++;
	lock.unlock();

	if (debugStream)
		*debugStream << "- " << *node << std::endl;
	
	++iterationCount;
	visitNode(node);

	delete node;
	return true;
}

void ThreadedPlanner9::operator()() {
	// HTN: loop
	while (step()) {}
}

void ThreadedPlanner9::pushNode(SearchNode* node) {
	if (debugStream)
		*debugStream << "+ " << *node << std::endl;

	boost::mutex::scoped_lock lock(mutex);
	nodes.insert(SearchNodes::value_type(getTotalCost(*node), node));
	condition.notify_one();
}

void ThreadedPlanner9::success(const Plan& plan) {
	boost::mutex::scoped_lock lock(mutex);
	plans.push_back(plan);
	condition.notify_all();
}
