#ifndef PLANNER9DISTRIBUTED_HPP_
#define PLANNER9DISTRIBUTED_HPP_

#include <QTcpServer>
#include "../core/planner9.hpp"

struct Domain;

enum Command {
	CMD_PROBLEM_SCOPE,
	CMD_PUSH_NODE,
	CMD_GET_NODE,
	CMD_PLAN,
	CMD_CURRENT_COST
};

struct Serializer: public QDataStream {
	Serializer(const Domain& domain);
	
	template<typename T>
	void write(const T& t) { *this << t; }
	void write(const Planner9::SearchNode*& node);
	
	template<typename T>
	T read() { T t; *this >> t; return t; }
	
	const Domain& domain;
};

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
