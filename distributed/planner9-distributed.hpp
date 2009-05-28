#ifndef PLANNER9DISTRIBUTED_HPP_
#define PLANNER9DISTRIBUTED_HPP_

#include <QTcpServer>
#include "serializer.hpp"

struct Domain;
class SimplePlanner9;

struct SlavePlanner9: QObject {
	
	Q_OBJECT
	
	SlavePlanner9(const Domain& domain);

protected slots:
	void newConnection();
	void disconnected();
	void networkDataAvailable();

protected:
	void timerEvent(QTimerEvent *event);
		
protected:
	int timerId;
	SimplePlanner9* planner;
	QTcpSocket *clientConnection;
	QTcpServer tcpServer;
	Serializer stream;
};


#endif // PLANNER9DISTRIBUTED_HPP_
