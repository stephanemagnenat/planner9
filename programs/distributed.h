#ifndef DISTRIBUTED_H_
#define DISTRIBUTED_H_

#include <QObject>
#include <QTime>
#include <fstream>

class Plan;
class MasterPlanner9;

class Dumper: public QObject {
	Q_OBJECT

public:
	Dumper(MasterPlanner9& masterPlanner, int maxRunCount);
	
public slots:
	void planningStarted();
	void planningSucceded(const Plan& plan);
	void planningFailed();
	void planningFinished(const unsigned& totalIterationsCount);

protected:
	QTime planStartTime;
	MasterPlanner9& masterPlanner;
	std::ofstream statsFile;
	int runCounter;
	int maxRunCount;
};
		

#endif // DISTRIBUTED_H_
