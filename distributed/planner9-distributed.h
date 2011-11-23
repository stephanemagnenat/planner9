#ifndef PLANNER9DISTRIBUTED_HPP_
#define PLANNER9DISTRIBUTED_HPP_

#include <QTcpServer>
#include <QSet>
#include <QTime>
#include <fstream>
#include "serializer.hpp"
#include "chunked.h"
#include "../core/problem.hpp"

struct Domain;
class SimplePlanner9;
class ChunkedDevice;
class AvahiServer;
class AvahiEntryGroup;
class AvahiServiceBrowser;

struct SlavePlanner9: QObject {

	Q_OBJECT

public:
	SlavePlanner9(const Domain& domain, std::ostream* debugStream = 0);
	~SlavePlanner9();

protected slots:
	void newConnection();
	void disconnected();
	void messageAvailable();

protected:
	size_t getBestsCount() const;
	Planner9::Cost getBestsMinCost() const;
	Planner9::Cost getBestsMaxCost() const;
	virtual void timerEvent(QTimerEvent *event);
	void runPlanner(const Scope& scope);
	void killPlanner();
	void runTimer();
	void stopTimer();

private:
	void registerService();
	void unregisterService();

private:
	int timerId;
	SimplePlanner9* planner;
	Planner9::CostFunction* costFunction;
	ChunkedDevice* device;
	QTcpServer tcpServer;
	Serializer stream;
	QTime lastSentCostTime;
	Planner9::Cost lastSentMinCost;
	Planner9::Cost lastSentMaxCost;
	std::ostream* debugStream;
	AvahiServer* avahiServer;
	AvahiEntryGroup* avahiEntryGroup;
};

struct MasterPlanner9: QObject {

	Q_OBJECT

	struct Client {
		Client();
		Client(ChunkedDevice* device);

		ChunkedDevice* device;
		Planner9::Cost bestsMinCost;
		Planner9::Cost bestsMaxCost;
		bool nodeRequested;
	};

public:
	MasterPlanner9(const Domain& domain, std::ostream* debugStream = 0);
	~MasterPlanner9();

	const Scope& getProblemScope() const { return problem.scope; }
	const Domain& getDomain() const { return stream.domain; }
	
	bool connectToSlave(const QString& hostName, quint16 port);

	void plan(const Problem& problem, Planner9::CostFunction* costFunction = 0);

public slots:
	// TODO: debug/bench only
	void replan();

signals:
	void planningStarted();
	void planningSucceded(const Plan& plan);
	void planningFailed();
	void planningFinished(const unsigned& totalIterationsCount);

protected slots:
	void clientConnected();
	void clientDisconnected();
	void clientConnectionError(QAbstractSocket::SocketError socketError);
	void clientDiscovered(int interface, int protocol, const QString &name, const QString &type, const QString &domain, uint flags);
    void messageAvailable();

protected:
	void processMessage(Client& client);
	void startSearchIfReady();
	void stopClients();
	bool isAnyClientSearching() const;

	void sendGetNode(ChunkedDevice* device);
	void sendScope(ChunkedDevice* client);
	void sendInitialNode(ChunkedDevice* client);
	void sendNode(ChunkedDevice* client, const SimplePlanner9::SearchNode& node);
	void sendStop(ChunkedDevice* client);

private:
	Problem problem;
	Planner9::CostFunction* costFunction;
	Planner9::SearchNode *initialNode;
	typedef QMap<QTcpSocket*, Client> ClientsMap;
	ClientsMap clients;
	Serializer stream;
	int stoppingCount;
	bool newSearch;
	unsigned totalIterationCount;
	std::ostream* debugStream;
	AvahiServer* avahiServer;
	AvahiServiceBrowser* avahiServiceBrowser;
};


#endif // PLANNER9DISTRIBUTED_HPP_
