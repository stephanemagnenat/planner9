#include "distributed.h"
#include "distributed.moc"
#include "../core/planner9.hpp"
#include "../distributed/planner9-distributed.h"
#include "../distributed/planner9-dbus.h"
//#include "../problems/robots.hpp"
//#include "problems/rover.hpp"
#include "../problems/rescue-numeric.hpp"
#include <QApplication>
#include <QTimer>
#include <QDBusMetaType>

using namespace std;

Dumper::Dumper(MasterPlanner9& masterPlanner, int maxRunCount) :
	masterPlanner(masterPlanner),
	statsFile("stats.txt"),
	runCounter(0),
	maxRunCount(maxRunCount)
{
	connect(&masterPlanner, SIGNAL(planningStarted()), SLOT(planningStarted()));
	connect(&masterPlanner, SIGNAL(planningSucceded(const Plan&)), SLOT(planningSucceded(const Plan&)));
	connect(&masterPlanner, SIGNAL(planningFailed()), SLOT(planningFailed()));
	connect(&masterPlanner, SIGNAL(planningFinished(const unsigned&)), SLOT(planningFinished(const unsigned&)));
}

void Dumper::planningStarted() {
	planStartTime = QTime::currentTime();
	std::cerr << "Starting planning" << std::endl;
}

void Dumper::planningSucceded(const Plan& plan) {
	const int planningDuration(planStartTime.msecsTo(QTime::currentTime()));
	std::cerr << Scope::setScope(masterPlanner.getProblemScope());
	std::cerr << "After " << planningDuration << " ms, plan:\n" << plan << std::endl;
	statsFile << planningDuration;
}

void Dumper::planningFailed() {
	const int planningDuration(planStartTime.msecsTo(QTime::currentTime()));
	std::cerr << "After " << planningDuration << " ms, no plan." << std::endl;
	statsFile << planningDuration;
}

void Dumper::planningFinished(const unsigned& totalIterationsCount) {
	std::cerr << "Planning finished, total Iterations " << totalIterationsCount << std::endl;
	statsFile << " " << totalIterationsCount << std::endl;
	if (maxRunCount) {
		sleep(1);
		runCounter++;
		std::cerr << "Redoing planning for statistics, run " << runCounter << std::endl;
		if (runCounter < maxRunCount) {
			masterPlanner.replan();
		} else {
			QCoreApplication::quit();
		}
	}
}

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
	int maxRunCount(0);
	if (argc >= 3)
		maxRunCount = atoi(argv[2]);
	Dumper dumper(masterPlanner, maxRunCount);
	
	MasterAdaptor::registerDBusTypes();
	
	new MasterAdaptor(&masterPlanner);
	
	/*
	start plan only using dbus now  
	masterPlanner.plan(problem);
	*/
	
	/*
	Use purely avahi now
	for (int i = 2; i < argc; i+=2) {
		masterPlanner.connectToSlave(argv[i], atoi(argv[i+1]));
	}
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
