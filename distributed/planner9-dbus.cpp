#include "planner9-dbus.h"
#include "planner9-dbus.moc"
#include "planner9-distributed.h"
#include <QDBusConnection>
#include <QDBusMetaType>
#include "../core/scope.hpp"


QDBusArgument &operator<<(QDBusArgument &argument, const DBusAtom &atom) {
	argument.beginStructure();
	argument << atom.relation << atom.params;
	argument.endStructure();
	return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusAtom &atom) {
	argument.beginStructure();
	argument >> atom.relation >> atom.params;
	argument.endStructure();
	return argument;
}

DBusTask::DBusTask() {
}

DBusTask::DBusTask(const Task& task) :
	head(QString::fromStdString(task.head->name)) {
	for (Variables::const_iterator it = task.params.begin(); it != task.params.end(); ++it) {
		this->params.push_back(it->index);
	}
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusTask &task) {
	argument.beginStructure();
	argument << task.head<< task.params;
	argument.endStructure();
	return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusTask &task) {
	argument.beginStructure();
	argument >> task.head >> task.params;
	argument.endStructure();
	return argument;
}

void MasterAdaptor::registerDBusTypes() {
	qDBusRegisterMetaType<DBusParams>();
	qDBusRegisterMetaType<DBusAtom>();
	qDBusRegisterMetaType<DBusState>();
	qDBusRegisterMetaType<DBusTask>();
	qDBusRegisterMetaType<DBusPlan>();
}

MasterAdaptor::MasterAdaptor(MasterPlanner9* master) :
	QDBusAbstractAdaptor(master),
	master(master) {
	
	connect(master, SIGNAL(planningStarted()), SIGNAL(PlanningStarted()));
	connect(master, SIGNAL(planningStarted()), SIGNAL(PlanningStarted()));
	connect(master, SIGNAL(planningSucceded(Plan)), SLOT(emitPlanningSucceeded(Plan)));
	connect(master, SIGNAL(planningFailed()), SIGNAL(PlanningFailed()));
	connect(master, SIGNAL(planningFinished(uint)), SIGNAL(PlanningFinished(uint)));
	
	QDBusConnection::sessionBus().registerObject("/", master);
	QDBusConnection::sessionBus().registerService("ch.epfl.mobots.Planner9");
}

void MasterAdaptor::emitPlanningSucceeded(const Plan& plan) {
	DBusPlan dbusPlan;
	
	for (Plan::const_iterator it = plan.begin(); it != plan.end(); ++it) {
		const Task& task(*it);
		dbusPlan.push_back(task);
	}
	
	emit PlanningSucceeded(dbusPlan);
}

static Variables fromDBusParams(const DBusParams& params) {
	Variables variables;
	variables.reserve(params.size());
	for (int i = 0; i<params.size(); ++i) {
		variables.push_back(Variable(params[i]));
	}
	return variables;
}

void MasterAdaptor::StartPlanning(const QStringList& constants, const DBusState& state, const DBusTask& task) {
	// Create problem with scope
	Scope scope;
	scope.names.reserve(constants.size());
	for (QStringList::const_iterator it = constants.begin(); it != constants.end(); ++it) {
		scope.names.push_back(it->toStdString());
	}
	Problem problem(scope);
	
	// Add state
	for (DBusState::const_iterator it = state.begin(); it != state.end(); ++it) {
		const QString& relationName(it->relation);
		const Relation* relation(master->getDomain().getRelation(relationName.toStdString()));
		problem.state.insert(Atom(relation, fromDBusParams(it->params)));
	}
	
	// Add initial task
	const QString& headName(task.head);
	const Head* head(master->getDomain().getHead(headName.toStdString()));
	problem.network.first.push_back(new TaskNetwork::Node(Task(head, fromDBusParams(task.params))));
	
	// Start planning
	master->plan(problem);
}


void MasterAdaptor::RestartPlanning()
{
	master->replan();
}

