#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include <QDataStream>
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

#endif // SERIALIZER_HPP_
