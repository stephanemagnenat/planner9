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

typedef QList<quint16> DBusParams;

Q_DECLARE_METATYPE(DBusParams)

struct DBusAtom {
	QString function;
	DBusParams params;
	QString value;
};
typedef QList<DBusAtom> DBusState;

Q_DECLARE_METATYPE(DBusAtom)
Q_DECLARE_METATYPE(DBusState)

struct DBusTask {
	QString head;
	DBusParams params;
};
typedef QList<DBusTask> DBusPlan;

Q_DECLARE_METATYPE(DBusTask)
Q_DECLARE_METATYPE(DBusPlan)

QDBusArgument &operator<<(QDBusArgument &argument, const DBusAtom &atom) {
	argument.beginStructure();
	argument << atom.function << atom.params << atom.value;
	argument.endStructure();
	return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusAtom &atom) {
	argument.beginStructure();
	argument >> atom.function >> atom.params >> atom.value;
	argument.endStructure();
	return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusTask &task) {
	argument.beginStructure();
	argument << task.head << task.params;
	argument.endStructure();
	return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusTask &task) {
	argument.beginStructure();
	argument >> task.head >> task.params;
	argument.endStructure();
	return argument;
}

void registerDBusTypes() {
	qDBusRegisterMetaType<DBusParams>();
	qDBusRegisterMetaType<DBusAtom>();
	qDBusRegisterMetaType<DBusState>();
	qDBusRegisterMetaType<DBusTask>();
	qDBusRegisterMetaType<DBusPlan>();
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

int main(int argc, char* argv[]) {
	QCoreApplication app(argc, argv);
	
	registerDBusTypes();
	
	QTextStream input(stdin);
	QString word;
	
	DBusState state;
	DBusTask task;
	
	// parse state
	eat(input, '(');
	while (true) {
		if (eatOneOf(input, '(', ')'))
			break;
		DBusAtom entry;
		input >> word;
		entry.function = word;
		input >> word;
		bool equalsFound(false);
		while (!word.endsWith(')')) {
			QStringList list(word.split("=", QString::SkipEmptyParts));
			if (!list.empty()) {
				if (!equalsFound) {
					entry.params.push_back(getIndex(list.at(0)));
					if (list.size() > 1)
						entry.value = list.at(1);
				} else {
					entry.value = list.at(0);
				}
			}
			equalsFound = word.contains("=") || equalsFound;
			input >> word;
		}
		if (word.size() > 1) {
			word.chop(1);
			QStringList list(word.split("=", QString::SkipEmptyParts));
			if (!list.empty()) {
				if (!equalsFound) {
					entry.params.push_back(getIndex(list.at(0)));
					if (list.size() > 1)
						entry.value = list.at(1);
				} else {
					entry.value = list.at(0);
				}
			}
			equalsFound = word.contains("=") || equalsFound;
		}
		state.push_back(entry);
	}
	
	// parse goal
	eat(input, '(');
	input >> word;
	if (word.endsWith(')')) {
		word.chop(1);
		task.head = word;
	} else {
		task.head = word;
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
