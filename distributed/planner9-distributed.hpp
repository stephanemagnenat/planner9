#ifndef PLANNER9DISTRIBUTED_HPP_
#define PLANNER9DISTRIBUTED_HPP_

#include <QTcpServer>
#include <QSet>
#include "serializer.hpp"
#include "../core/problem.hpp"

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
	virtual void timerEvent(QTimerEvent *event);
	void killPlanner();
		
protected:
	int timerId;
	SimplePlanner9* planner;
	QTcpSocket *clientConnection;
	QTcpServer tcpServer;
	Serializer stream;
};

struct MasterPlanner9: QObject {
	
	Q_OBJECT
	
	MasterPlanner9(const Domain& domain);
	~MasterPlanner9();
	
	bool connectToSlave(const QString& hostName, quint16 port);
	
	void plan(const Problem& problem);
	
protected:	// TODO: do we inherit or use socket/slot ?
	virtual void planFound(const Plan& plan);
	virtual void noPlanFound();

protected slots:
	void clientConnected();
	void clientDisconnected();
	void clientConnectionError(QAbstractSocket::SocketError socketError);
	void networkDataAvailable();
	
protected:
	void sendScope(QTcpSocket* client);
	void sendInitialNode(QTcpSocket* client);
	void sendNode(QTcpSocket* client, const SimplePlanner9::SearchNode& node);
	void sendStop(QTcpSocket* client);
	
protected:
	Problem problem;
	Planner9::SearchNode *initialNode;
	typedef QMap<QTcpSocket*, Planner9::Cost> ClientsMap;
	ClientsMap clients;
	Serializer stream;
};


#endif // PLANNER9DISTRIBUTED_HPP_
