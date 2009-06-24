//#include "distributed-client.h"
#include <QCoreApplication>
#include <QDBusInterface>
#include <QStringList>
#include <QMap>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QTextStream>
#include <QtDebug>
#include <cstdio>
#include <iostream>

struct NamedList
{
	QString name;
	QList<quint16> params;
};
typedef QList<NamedList> NamedListVector;

Q_DECLARE_METATYPE(NamedList)
Q_DECLARE_METATYPE(NamedListVector)

QDBusArgument &operator<<(QDBusArgument &argument, const NamedList &entry) {
	argument.beginStructure();
	argument << entry.name << entry.params;
	argument.endStructure();
	return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NamedList &entry) {
	argument.beginStructure();
	argument >> entry.name >> entry.params;
	argument.endStructure();
	return argument;
}

// support functions for parsing
void eat(QTextStream& stream, const QChar& c) {
	stream.skipWhiteSpace();
	QChar v;
	stream >> v;
	if (v == c)
		return;
	std::cerr << QString("Expected character %0, got character %1 instead").arg(c).arg(v).constData();
	abort();
}

int eatOneOf(QTextStream& stream, const QChar& c1, const QChar& c2) {
	stream.skipWhiteSpace();
	QChar v;
	stream >> v;
	if (v == c1)
		return 0;
	if (v == c2)
		return 1;
	std::cerr << QString("Expected character %0 or %1, got character %1 instead").arg(c1).arg(c2).arg(v).constData();
	abort();
}

// data
QStringList constantsNames;
QMap<QString, quint16> constantsMap;

// support functions for data
quint16 getIndex(const QString& s) {
	QMap<QString, quint16>::const_iterator it = constantsMap.find(s);
	if (it != constantsMap.end()) {
		return it.value();
	} else {
		const quint16 index(constantsNames.size());
		constantsMap[s] = index;
		constantsNames.push_back(s);
		return index;
	}
}

/*
 
// for robots
(
	(object o0)
	(object o1)
	(robots r0)
	(robots r1)
	(resource nut0)
	(resource nut1)
	(area a0)
	(area a1)
	(area a2)
	(area a3)
	(area a4)
	(area a5)
	(area a6)
	(isConnectable a0 a5)
	(isConnectable a0 a1)
	(isConnectable a1 a2)
	(isConnectable a2 a3)
	(isConnectable a2 a6)
	(isConnectable a3 a4)
	(isIn o0 a0)
	(isIn o1 a2)
	(isIn r0 a1)
	(isIn r1 a6)
	(isIn nut0 a1)
	(isIn nut1 a6)
)
(move o0 a4)

// for rescue

(
	 (object o0)
	 (robots r0)
	 (robots r1)
	 (area a0)
	 (area a1)
	 (area a2)
	 (area a3)
	 (area a4)
	 (area a5)
	 (isAdjacent a0 a1)
	 (isAdjacent a0 a3)
	 (isAdjacent a1 a2)
	 (isAdjacent a1 a3)
	 (isAdjacent a1 a4)
	 (isAdjacent a4 a5)
	 (isIn o0 a0)
	 (isIn r0 a0)
	 (isIn r1 a1)
	 (extinguisher ext0)
	 (extinguisher ext1)
	 (extinguisher ext2)
	 (extinguisher ext3)
	 (isIn ext0 a0)
	 (isIn ext1 a1)
	 (isIn ext2 a2)
	 (isIn ext3 a2)
)
(rescue a5)
  
  */

int main(int argc, char* argv[]) {
	QCoreApplication app(argc, argv);
	
	qDBusRegisterMetaType<NamedList>();
	qDBusRegisterMetaType<NamedListVector>();
	
	QTextStream input(stdin);
	QString word;
	
	NamedListVector state;
	NamedList task;
	
	// parse state
	eat(input, '(');
	while (true) {
		if (eatOneOf(input, '(', ')'))
			break;
		NamedList entry;
		input >> word;
		entry.name = word;
		input >> word;
		while (!word.endsWith(')')) {
			entry.params.push_back(getIndex(word));
			input >> word;
		}
		if (word.size() > 1) {
			word.chop(1);
			entry.params.push_back(getIndex(word));
		}
		state.push_back(entry);
	}
	
	// parse goal
	eat(input, '(');
	input >> word;
	if (word.endsWith(')')) {
		word.chop(1);
		task.name = word;
	} else {
		task.name = word;
		input >> word;
		while (!word.endsWith(')')) {
			task.params.push_back(getIndex(word));
			input >> word;
		}
		if (word.size() > 1) {
			word.chop(1);
			task.params.push_back(getIndex(word));
		}
	}
		
	qDebug() << "parsing done";
	
	// call D-Bus
	QDBusInterface planner("ch.epfl.mobots.Planner9", "/", "ch.epfl.mobots.HTNPlanner");
	planner.call("StartPlanning", qVariantFromValue(constantsNames), qVariantFromValue(state), qVariantFromValue(task));
	
	return 0;
}
