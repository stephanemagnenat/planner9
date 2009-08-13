#ifndef PLANNER9DBUS_H_
#define PLANNER9DBUS_H_

#include <QDBusAbstractAdaptor>
#include <QMetaType>

class MasterPlanner9;
class Plan;
class Task;
class Scope;

typedef QList<quint16> DBusParams;

struct DBusAtom {
	QString relation;
	DBusParams params;
	QString value;
};
typedef QList<DBusAtom> DBusState;

struct DBusTask {
	QString head;
	DBusParams params;
	
	DBusTask();
	DBusTask(const Task& task);
};
typedef QList<DBusTask> DBusPlan;

class MasterAdaptor: public QDBusAbstractAdaptor {
	
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "ch.epfl.mobots.HTNPlanner")

private:
	MasterPlanner9* master;
	
public:
	static void registerDBusTypes();
	
	MasterAdaptor(MasterPlanner9* master);
	
private slots:
	void emitPlanningSucceeded(const Plan& plan);
	
public slots:
	Q_NOREPLY void StartPlanning(const QStringList& constants, const DBusState& state, const DBusTask& task);
	Q_NOREPLY void RestartPlanning();

signals:
	void PlanningStarted();
	void PlanningSucceeded(const DBusPlan& plan);
	void PlanningFailed();
	void PlanningFinished(const unsigned& totalIterationsCount);
};

Q_DECLARE_METATYPE(DBusParams)
Q_DECLARE_METATYPE(DBusAtom)
Q_DECLARE_METATYPE(DBusState)
Q_DECLARE_METATYPE(DBusTask)
Q_DECLARE_METATYPE(DBusPlan)



#endif // PLANNER9DBUS_H_
