#include "../core/planner9.hpp"
#include "../distributed/planner9-distributed.h"
#include "../problems/robots.hpp"
//#include "problems/rover.hpp"
#include <QApplication>

using namespace std;

int dumpError(char *exeName) {
	std::cerr << "Error, usage " << exeName << " slave/master SLAVE0HOST SLAVE0PORT ... SLAVE1HOST SLAVE1PORT" << std::endl;
	return 1;
}

int runSlave(int argc, char* argv[]) {
	QApplication app(argc, argv);
	
	MyProblem problem;
	
	//SlavePlanner9 slavePlanner(problem, &std::cerr);
	SlavePlanner9 slavePlanner(problem);
	
	return app.exec();
}

int runMaster(int argc, char* argv[]) {
	QApplication app(argc, argv);
	
	MyProblem problem;
	
	MasterPlanner9 masterPlanner(problem, &std::cerr);
	
	masterPlanner.plan(problem);
	
	for (int i = 2; i < argc; i+=2) {
		masterPlanner.connectToSlave(argv[i], atoi(argv[i+1]));
	}
	
	return app.exec();
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		return dumpError(argv[0]);
	}

	if (strcmp(argv[1], "slave") == 0)
		return runSlave(argc, argv);
	else if (strcmp(argv[1], "master") == 0)
		return runMaster(argc, argv);
	else
		return dumpError(argv[0]);
}