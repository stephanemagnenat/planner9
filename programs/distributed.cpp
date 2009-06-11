#include "../core/planner9.hpp"
#include "../distributed/planner9-distributed.h"
#include "../distributed/planner9-dbus.h"
#include "../problems/robots.hpp"
//#include "problems/rover.hpp"
#include <QApplication>
#include <QTimer>
#include <QDBusMetaType>

using namespace std;
/*
void MasterPlanner9::planFound(const Plan& plan) {
	const int planningDuration(planStartTime.msecsTo(QTime::currentTime()));
	std::cerr << "After " << planningDuration << " ms, plan:\n" << plan << std::endl;
	statsFile << planningDuration;
}

void MasterPlanner9::noPlanFound() {
	const int planningDuration(planStartTime.msecsTo(QTime::currentTime()));
	std::cerr << "After " << planningDuration << " ms, no plan." << std::endl;
	statsFile << planningDuration;
}


	qDebug() << "Total Iterations" << totalIterationCount;
	statsFile << " " << totalIterationCount << std::endl;
	
	

	planStartTime = QTime::currentTime();
	
	qDebug() << "Starting planning";
	
	
	
	QTime planStartTime;
	
	
	std::ofstream statsFile; // TODO cleanup this
	
	statsFile("stats.txt")
	*/

int dumpError(char *exeName) {
	std::cerr << "Error, usage " << exeName << " slave/master SLAVE0HOST SLAVE0PORT ... SLAVE1HOST SLAVE1PORT" << std::endl;
	return 1;
}

int runSlave(int argc, char* argv[]) {
	QCoreApplication app(argc, argv);
	
	MyProblem problem;
	
	//SlavePlanner9 slavePlanner(problem, &std::cerr);
	SlavePlanner9 slavePlanner(problem);
	
	return app.exec();
}

int runMaster(int argc, char* argv[]) {
	QCoreApplication app(argc, argv);
	
	MyDomain domain;
	
	//MasterPlanner9 masterPlanner(problem, &std::cerr);
	MasterPlanner9 masterPlanner(domain);
	
	MasterAdaptor::registerDBusTypes();
	
	new MasterAdaptor(&masterPlanner);
	
	//masterPlanner.plan(problem);
	
	for (int i = 2; i < argc; i+=2) {
		masterPlanner.connectToSlave(argv[i], atoi(argv[i+1]));
	}

	/*// QTimer for statisticaly significant benchs
	QTimer* replanTimer(new QTimer(&masterPlanner));
	masterPlanner.connect(replanTimer, SIGNAL(timeout()), SLOT(replan()));
	replanTimer->setInterval(40000);
	
	// QTimer  for total duration
	QTimer* totalTimer(new QTimer(&masterPlanner));
	app.connect(totalTimer, SIGNAL(timeout()), SLOT(quit()));
	totalTimer->setInterval(40000 * 20 + 30000);
	
	// run timers
	totalTimer->start();
	replanTimer->start();
	*/
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
