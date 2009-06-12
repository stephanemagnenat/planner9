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
	Dumper(const MasterPlanner9& masterPlanner);
	
public slots:
	void planningStarted();
	void planningSucceded(const Plan& plan);
	void planningFailed();
	void planningFinished(const unsigned& totalIterationsCount);

protected:
	QTime planStartTime;
	const MasterPlanner9& masterPlanner;
	std::ofstream statsFile;
};
		

#endif // DISTRIBUTED_H_
