#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include <QDataStream>
#include "../core/planner9.hpp"

enum Command {
	CMD_PROBLEM_SCOPE,
	CMD_PUSH_NODE,
	CMD_GET_NODE,
	CMD_PLAN_FOUND,
	CMD_NOPLAN_FOUND,
	CMD_CURRENT_COST,
	CMD_STOP
};

struct Domain;

struct Serializer: public QDataStream {
	Serializer(const Domain& domain);
	Serializer(QIODevice * d, const Domain& domain);
	
	template<typename T>
	void write(const T& t) { *this << t; }
	
	template<typename T>
	T read() { T t; *this >> t; return t; }
	
	const Domain& domain;
};
/*
TODO: examples for safe network operations
bool sendCurrentCost(const Domain& domain, QAbstractSocket* socket, const Planner9::Cost& cost);
Cost getCurrentCost(const Domain& domain, QAbstractSocket* socket, bool* ok);

*/

#endif // SERIALIZER_HPP_
