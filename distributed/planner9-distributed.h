#ifndef PLANNER9DISTRIBUTED_HPP_
#define PLANNER9DISTRIBUTED_HPP_

#include <QTcpServer>
#include <QSet>
#include <QTime>
#include "serializer.hpp"
#include "chunked.h"
#include "../core/problem.hpp"

struct Domain;
class SimplePlanner9;
class ChunkedDevice;

struct SlavePlanner9: QObject {

	Q_OBJECT

public:
	SlavePlanner9(const Domain& domain, std::ostream* debugStream = 0);

protected slots:
	void newConnection();
	void disconnected();
	void messageAvailable();

protected:
	virtual void timerEvent(QTimerEvent *event);
	void runPlanner(const Scope& scope);
	void killPlanner();
	void runTimer();
	void stopTimer();

private:
	int timerId;
	SimplePlanner9* planner;
	ChunkedDevice* device;
	QTcpServer tcpServer;
	Serializer stream;
	QTime lastSentCostTime;
	Planner9::Cost lastSentCost;
	std::ostream* debugStream;
};

struct MasterPlanner9: QObject {

	Q_OBJECT

	struct Client {
		Client();
		Client(ChunkedDevice* device);

		ChunkedDevice* device;
		Planner9::Cost cost;
	};

public:
	MasterPlanner9(const Domain& domain, std::ostream* debugStream = 0);
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
	void messageAvailable();

protected:
	void processMessage(Client& client);
	void startSearchIfReady();
	void stopClients();

	void sendGetNode(ChunkedDevice* device);
	void sendScope(ChunkedDevice* client);
	void sendInitialNode(ChunkedDevice* client);
	void sendNode(ChunkedDevice* client, const SimplePlanner9::SearchNode& node);
	void sendStop(ChunkedDevice* client);

private:


	Problem problem;
	Planner9::SearchNode *initialNode;
	typedef QMap<QTcpSocket*, Client> ClientsMap;
	ClientsMap clients;
	Serializer stream;
	QTime planStartTime;
	int stoppingCount;
	bool newSearch;
	std::ostream* debugStream;
};


#endif // PLANNER9DISTRIBUTED_HPP_
