#include "planner9-distributed.h"
#include "../core/relations.hpp"
#include "../core/planner9.hpp"
#include "../core/tasks.hpp"
#include "../core/costs.hpp"
#include <boost/cast.hpp>
#include <QTcpSocket>
#include "avahi-server.h"
#include "avahi-entry-group.h"
#include "avahi-service-browser.h"
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <stdexcept>

#include "planner9-distributed.moc"


// force the use of specialized versions

template<> void Serializer::write(const Command& cmd);
template<> void Serializer::write(const Scope& scope);
template<> void Serializer::write(const Task& task);
template<> void Serializer::write(const Plan& plan);
template<> void Serializer::write(const Atom& atom);
template<> void Serializer::write(const CNF& cnf);
template<> void Serializer::write(const State& state);
template<> void Serializer::write(const TaskNetwork& network);
template<> void Serializer::write(const Planner9::SearchNode& node);

template<> Command Serializer::read();
template<> Scope Serializer::read();
template<> Task Serializer::read();
template<> Plan Serializer::read();
template<> Atom Serializer::read();
template<> CNF Serializer::read();
template<> State Serializer::read();
template<> TaskNetwork Serializer::read();
template<> Planner9::SearchNode Serializer::read();

//TODO: tune
const size_t toBalanceCount = 10;
const size_t toBalanceRatio = 3;

static AlternativesCost alternativesCost;

SlavePlanner9::SlavePlanner9(const Domain& domain, std::ostream* debugStream):
	timerId(-1),
	planner(0),
	costFunction(&alternativesCost), // TODO: get cost function from networks
	device(0),
	stream(domain),
	debugStream(debugStream),
	avahiServer(new AvahiServer("org.freedesktop.Avahi", "/", QDBusConnection::systemBus(), this)),
	avahiEntryGroup(0) {

	tcpServer.setMaxPendingConnections(1);

	connect(&tcpServer, SIGNAL(newConnection()), SLOT(newConnection()));

	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}

	if (debugStream) *debugStream << "Listening on port " << tcpServer.serverPort() << std::endl;

	registerService();
}

SlavePlanner9::~SlavePlanner9() {
	unregisterService();
}

void SlavePlanner9::newConnection() {
	Q_ASSERT(device == 0);

	QTcpSocket* socket = tcpServer.nextPendingConnection();
	Q_ASSERT(socket);
	tcpServer.close();

	device = new ChunkedDevice(socket);
	stream.setDevice(device);

	connect(device, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(device, SIGNAL(readyRead()), SLOT(messageAvailable()));

	if (debugStream) *debugStream << "Connection from " << socket->peerAddress().toString().toStdString() << std::endl;

	unregisterService();
}

void SlavePlanner9::disconnected() {
	stopTimer();
	killPlanner();

	if (debugStream) *debugStream << "Connection closed"  << std::endl;

	device->deleteLater();
	device = 0;

	if (!tcpServer.listen()) {
		throw std::runtime_error(tcpServer.errorString().toStdString());
	}

	if (debugStream) *debugStream << "Listening on port " << tcpServer.serverPort() << std::endl;

	registerService();
}

void SlavePlanner9::messageAvailable() {
	while (device->isMessage()) {
		// fetch command from master
		Command cmd(stream.read<Command>());

		qDebug() << "\n*" << commandsNames[cmd];

		switch (cmd) {
			// new node to insert
			case CMD_PUSH_NODE: {
				assert(planner);
				const size_t toReceiveCount(stream.read<quint32>());
				qDebug() << "inserting" << toReceiveCount << "nodes";
				for (size_t i = 0; i < toReceiveCount; ++i) {
					SimplePlanner9::SearchNode* node(new SimplePlanner9::SearchNode(stream.read<SimplePlanner9::SearchNode>()));
					planner->pushNode(node);
				}
				runTimer();
			} break;

			// node to send
			case CMD_GET_NODE: {
				if (planner) {
					stream.write(CMD_PUSH_NODE);
					const size_t toSendCount(getBestsCount());
					qDebug() << "Sending" << toSendCount << "nodes on" << planner->nodes.size();
					stream.write<quint32>(toSendCount);
					for (size_t i = 0; i < toSendCount; ++i) {
						const Planner9::SearchNode* node(planner->nodes.begin()->second);
						stream.write(*node);
						qDebug() << "cost " << node->getTotalCost() << " - path " << node->pathCost << ", heuristic " << node->heuristicCost;
						delete node;
						planner->nodes.erase(planner->nodes.begin());
						//if (debugStream) *debugStream  << "sending:\n" << *(planner->nodes.begin()->second) << "\ndone" << std::endl;
					}
					device->flush();
				}
			} break;

			// new problem scope
			case CMD_PROBLEM_SCOPE: {
				killPlanner();
				const Scope scope(stream.read<Scope>());
				// TODO: get cost function
				runPlanner(scope);
			} break;

			// stop processing
			case CMD_STOP: {
				stopTimer();
				qDebug() << "Stopped after" << planner->iterationCount << "iterations";
				// acknowledge stop
				stream.write(CMD_STOP);
				stream.write<quint32>(planner->iterationCount);
				device->flush();
				// delete planner
				killPlanner();
			} break;

			default:
				throw std::runtime_error(tr("Unknown command received: %0").arg(cmd).toStdString());
				break;
		}
	}
}

size_t SlavePlanner9::getBestsCount() const {
	if ((planner == 0) || planner->nodes.empty())
		return 0;
	
	size_t toSendCount = planner->nodes.size() / toBalanceRatio;
	toSendCount = std::min(toSendCount, toBalanceCount);
	
	return toSendCount;
}

Planner9::Cost SlavePlanner9::getBestsMinCost() const {
	if ((planner == 0) || planner->nodes.empty())
		return Planner9::InfiniteCost;
	
	return planner->nodes.begin()->first;
}

Planner9::Cost SlavePlanner9::getBestsMaxCost() const {
	if ((planner == 0) || planner->nodes.empty())
		return Planner9::InfiniteCost;
	
	const size_t toSendCount(getBestsCount());
	
	if (toSendCount == 0)
		return Planner9::InfiniteCost;
	
	SimplePlanner9::SearchNodes::const_iterator nodeIt(planner->nodes.begin());
	for (size_t i = 1; i < toSendCount; ++i)
		++nodeIt;
	
	return nodeIt->first;
}

void SlavePlanner9::timerEvent(QTimerEvent *event) {
	Q_ASSERT(planner);

	// TODO: improve the performances of this loop by using a thread
	// and not polling of the event loop
	// plan for a specified duration
	const bool cont = planner->plan(1);

	if (cont) {
		// check for periodical update of cost
		const QTime currentTime(QTime::currentTime());
		if (lastSentCostTime.msecsTo(currentTime) > 30) {
			const Planner9::Cost currentMinCost(getBestsMinCost());
			const Planner9::Cost currentMaxCost(getBestsMaxCost());
			if (currentMinCost != lastSentMinCost || currentMaxCost != lastSentMaxCost) {
				qDebug() << "\n* new current costs" << currentMinCost << currentMaxCost;
				stream.write(CMD_CURRENT_COST);
				stream.write(currentMinCost);
				stream.write(currentMaxCost);
				device->flush();
				
				lastSentMinCost = currentMinCost;
				lastSentMaxCost = currentMaxCost;
				lastSentCostTime = currentTime;
			}
		}
	} else {
		if (planner->plans.empty()) {
			// no more nodes, report failure
			qDebug() << "\n* no plan found";
			stream.write(CMD_NOPLAN_FOUND);
			device->flush();
		} else {
			// report plan
			qDebug() << "\n* plan found";
			for (SimplePlanner9::Plans::const_iterator it = planner->plans.begin(); it != planner->plans.end(); ++it) {
				stream.write(CMD_PLAN_FOUND);
				stream.write(planner->plans.front());
				device->flush();
			}
		}
		stopTimer();
	}
}


void SlavePlanner9::runPlanner(const Scope& scope) {
	Q_ASSERT(planner == 0);
	costFunction = &alternativesCost;
	planner = new SimplePlanner9(scope, costFunction, debugStream);
	lastSentMinCost = Planner9::InfiniteCost;
	lastSentMaxCost = Planner9::InfiniteCost;
	lastSentCostTime = QTime::currentTime();
}

void SlavePlanner9::killPlanner() {
	if (planner) {
		delete planner;
		planner = 0;
		// TODO: get cost function from networks
		/*if (costFunction) {
			delete costFunction;
			costFunction = 0;
		}*/
	}
}

void SlavePlanner9::runTimer() {
	if(timerId == -1) {
		timerId = startTimer(0);
	}
}

void SlavePlanner9::stopTimer() {
	if (timerId != -1) {
		killTimer(timerId);
		timerId = -1;
	}
}

void SlavePlanner9::registerService() {
	Q_ASSERT(avahiEntryGroup == 0);

	QString serviceName(QString("planner9:%0").arg(tcpServer.serverPort()));

	QDBusObjectPath groupPath(avahiServer->EntryGroupNew());
	avahiEntryGroup = new AvahiEntryGroup("org.freedesktop.Avahi", groupPath.path(), QDBusConnection::systemBus(), this);
	avahiEntryGroup->AddService(AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, serviceName, "_planner9._tcp", "", "", tcpServer.serverPort(), QList<QByteArray>());
	avahiEntryGroup->Commit().waitForFinished();
}

void SlavePlanner9::unregisterService() {
	assert(avahiEntryGroup != 0);
	avahiEntryGroup->Free().waitForFinished();
	avahiEntryGroup->deleteLater();
	avahiEntryGroup = 0;
}

/////

MasterPlanner9::Client::Client() :
	device(0),
	bestsMinCost(Planner9::InfiniteCost),
	bestsMaxCost(Planner9::InfiniteCost),
	nodeRequested(false) {
}

MasterPlanner9::Client::Client(ChunkedDevice* device) :
	device(device),
	bestsMinCost(Planner9::InfiniteCost),
	bestsMaxCost(Planner9::InfiniteCost),
	nodeRequested(false) {
}

MasterPlanner9::MasterPlanner9(const Domain& domain, std::ostream* debugStream):
	costFunction(&alternativesCost),
	initialNode(0),
	stream(domain),
	stoppingCount(0),
	newSearch(false),
	totalIterationCount(0),
	debugStream(debugStream),
	avahiServer(new AvahiServer("org.freedesktop.Avahi", "/", QDBusConnection::systemBus(), this)),
	avahiServiceBrowser(0) {

	QDBusObjectPath browserPath(avahiServer->ServiceBrowserNew(AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_planner9._tcp", "", 0));
	avahiServiceBrowser = new AvahiServiceBrowser("org.freedesktop.Avahi", browserPath.path(), QDBusConnection::systemBus(), this);
	connect(avahiServiceBrowser,
		SIGNAL(ItemNew(int, int, const QString &, const QString &, const QString &, uint)),
		SLOT(clientDiscovered(int, int, const QString &, const QString &, const QString &, uint))
	);
}

MasterPlanner9::~MasterPlanner9() {
	for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		QTcpSocket* client(it.key());
		client->disconnectFromHost();
	}
	if (initialNode)
		delete initialNode;
}

bool MasterPlanner9::connectToSlave(const QString& hostName, quint16 port) {
	QTcpSocket* client(new QTcpSocket);
	client->connectToHost(hostName, port);
	connect(client, SIGNAL(connected()), SLOT(clientConnected()));
	connect(client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
	connect(client, SIGNAL(disconnected()), client, SLOT(deleteLater()));
	connect(client, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(clientConnectionError(QAbstractSocket::SocketError)));
	connect(client, SIGNAL(error(QAbstractSocket::SocketError)), client, SLOT(deleteLater()));
	return true;
}

void MasterPlanner9::plan(const Problem& problem, Planner9::CostFunction* costFunction) {
	if (debugStream) {
		*debugStream << Scope::setScope(problem.scope);
		*debugStream << "initial state: "<< problem.state << std::endl;
		*debugStream << "initial network: " << problem.network << std::endl;
	}
	std::cerr << Scope::setScope(problem.scope);
	this->problem = problem;

	if (initialNode)
		delete initialNode;
	initialNode = new Planner9::SearchNode(Plan(), problem.network, problem.scope.getSize(), CNF(), problem.state, 0, costFunction);
	this->costFunction = costFunction;
	
	replan();
}

void MasterPlanner9::replan() {
	assert(initialNode);
	
	newSearch = true;

	if (isAnyClientSearching())
		stopClients();

	startSearchIfReady();
}


void MasterPlanner9::clientConnected() {
	QTcpSocket* socket(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	ChunkedDevice* device = new ChunkedDevice(socket);
	connect(device, SIGNAL(readyRead()), SLOT(messageAvailable()));

	clients[socket] = Client(device);

	if (debugStream) *debugStream << "New client" << device;

	if (initialNode) {
		sendScope(device);
		// TODO: send user cost function... describe which to use in a user-friendly way, enum?
	
		if (clients.size() == 1) {
			if (debugStream) *debugStream << "Sending:\n" << *initialNode << "\ndone" << std::endl;
			sendInitialNode(device);
		}
	}
}

void MasterPlanner9::clientDisconnected() {
	QTcpSocket* client(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	clients.remove(client);

	// TODO: manage disconnection, resend node of this one
	if (stoppingCount > 0)
		--stoppingCount;
}

void MasterPlanner9::clientConnectionError(QAbstractSocket::SocketError socketError) {
	QTcpSocket* client(boost::polymorphic_downcast<QTcpSocket*>(sender()));
	clients.remove(client);

	// TODO: report the error
}

void MasterPlanner9::clientDiscovered(int interface, int protocol, const QString &name, const QString &type, const QString &domain, uint flags) {

	int outProtocol;
	QString outName;
	QString outType;
	QString outDomain;
	QString outHost;
	int outAProtocol;
	QString outAddress;
	ushort outPort;
	QList<QByteArray> outTxt;
	uint outFlags;

	avahiServer->ResolveService(interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, outProtocol, outName, outType, outDomain, outHost, outAProtocol, outAddress, outPort, outTxt, outFlags);

	connectToSlave(outHost, outPort);
}

void MasterPlanner9::messageAvailable() {
	ChunkedDevice* device(boost::polymorphic_downcast<ChunkedDevice*>(sender()));
	QTcpSocket* socket = boost::polymorphic_downcast<QTcpSocket*>(device->parentDevice());
	Client& client(clients[socket]);

	while (device->isMessage()) {
		processMessage(client);
	}
}

void MasterPlanner9::processMessage(Client& client) {
	// fetch command from client
	//qDebug() << "Cmd pre";
	stream.setDevice(client.device);
	Command cmd(stream.read<Command>());
	qDebug() << "\n*" << QTime::currentTime().toString("hh:mm:ss:zzz") << commandsNames[cmd] << (void*)(client.device);

	switch (cmd) {
		// cost
		case CMD_CURRENT_COST: {
			const Planner9::Cost bestsMinCost(stream.read<Planner9::Cost>());
			const Planner9::Cost bestsMaxCost(stream.read<Planner9::Cost>());

			// ignore if stopping
			if (stoppingCount > 0)
				return;
			
			client.bestsMinCost = bestsMinCost;
			client.bestsMaxCost = bestsMaxCost;
			
			// do not take action if get node is sent
			if (client.nodeRequested)
				return;
			
			std::cerr << "Cost map: ";
			for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
				std::cerr << it.value().device << ": " << it.value().bestsMinCost << " " << it.value().bestsMaxCost << "\t";
			}
			std::cerr << std::endl;

			// only one client, return
			if (clients.size() <= 1)
				return;

			// read the clients map to see whether anyone has lower or higher cost
			bool hasHigher(false);
			for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
				const Planner9::Cost thatBestsMinCost(it.value().bestsMinCost);
				//const Planner9::Cost thatBestsMaxCost(it.value().bestsMaxCost);
				if (thatBestsMinCost < bestsMinCost)
					return; // another has lower cost, we are not optimal, return
				if (it.value().bestsMinCost > bestsMaxCost)
					hasHigher = true; // another has higher cost, request some of our bests
			}

			if (hasHigher) {
				std::cerr  << "Requesting nodes from " << client.device << std::endl;
				// we have lowest cost and someone has higher, get some of our best nodes
				sendGetNode(client.device);
				client.nodeRequested = true;
			}
		} break;

		// node
		case CMD_PUSH_NODE: {
			const size_t nodesCount(stream.read<quint32>());
			std::cerr  << "Receiving " << nodesCount << " nodes" << std::endl;
			
			// clear get node lock
			client.nodeRequested = false;
			
			ClientsMap::iterator highestCostIt(clients.end());
			ChunkedDevice* destDevice(0);
			
			// read nodes
			for (size_t i = 0; i < nodesCount; ++i) {
				stream.setDevice(client.device);
				SimplePlanner9::SearchNode node(stream.read<SimplePlanner9::SearchNode>());

				// ignore if stopping
				if (stoppingCount > 0)
					continue;
	
				// ignore only one client
				if (clients.size() <= 1)
					continue;

				// find the client with the worst cost that will receive our nodes
				if (highestCostIt == clients.end()) {
					Planner9::Cost highestMinCost(0);
					Planner9::Cost highestMaxCost(0);
			
					for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
						const Planner9::Cost minCost(it.value().bestsMinCost);
						const Planner9::Cost maxCost(it.value().bestsMaxCost);
						if (minCost > highestMinCost) {
							highestMinCost = minCost;
							highestMaxCost = maxCost;
							highestCostIt = it;
						}
						else if (minCost == highestMinCost && maxCost > highestMaxCost) {
							highestMaxCost = maxCost;
							highestCostIt = it;
						}
					}
					destDevice = highestCostIt.value().device;
					std::cerr  << "Load balancing " << client.device << " to " << destDevice <<  std::endl;
					
					stream.setDevice(destDevice);
					stream.write(CMD_PUSH_NODE);
					stream.write<quint32>(nodesCount);
				}
				
				// send the node to it and update cost
				if (debugStream) *debugStream  << "sending:\n" << node << "\ndone" << std::endl;
				
				stream.setDevice(destDevice);
				stream.write(node);
				highestCostIt.value().bestsMinCost = std::min(node.getTotalCost(), highestCostIt.value().bestsMinCost);
			}
			
			if (destDevice)
				destDevice->flush();
		} break;

		// plan
		case CMD_PLAN_FOUND: {
			const Plan plan(stream.read<Plan>());

			// ignore if stopping
			if (stoppingCount > 0)
				return;
			
			stopClients();

			// print the plan
			emit planningSucceded(plan);
		} break;

		// no plan for this client
		case CMD_NOPLAN_FOUND: {

			// ignore if stopping
			if (stoppingCount > 0)
				return;

			client.bestsMinCost = Planner9::InfiniteCost;
			client.bestsMaxCost = Planner9::InfiniteCost;

			if (isAnyClientSearching() == false) {
				// notify failure
				emit planningFailed();
				stopClients();
			}
		} break;

		// stop acknowledge
		case CMD_STOP: {
			client.bestsMinCost = Planner9::InfiniteCost;
			client.bestsMaxCost = Planner9::InfiniteCost;
			
			totalIterationCount += stream.read<quint32>();
			stoppingCount--;
			if (stoppingCount == 0) {
				emit planningFinished(totalIterationCount);
			}
			startSearchIfReady();
		} break;

		default:
			throw std::runtime_error(tr("Unknown command received: %0").arg(cmd).toStdString());
			break;
	}
}

void MasterPlanner9::startSearchIfReady() {
	// ignore if stopping
	if (stoppingCount > 0)
		return;

	// do not redo the same search twice
	if (!newSearch)
		return;
	newSearch = false;
	
	emit planningStarted();

	// send scope to every client for new planning
	for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
		Client& client = it.value();
		sendScope(client.device);
	}

	// if one client is present
	if (!clients.empty()) {
		sendInitialNode(clients.begin().value().device);
	}
}

void MasterPlanner9::stopClients() {
	// tell all clients to stop searching
	for (ClientsMap::iterator it = clients.begin(); it != clients.end(); ++it) {
		Client& client = it.value();
		client.bestsMinCost = Planner9::InfiniteCost;
		client.bestsMaxCost = Planner9::InfiniteCost;
		client.nodeRequested = false;
		sendStop(client.device);
	}

	// wait until all clients have acknowledged the stop
	stoppingCount = clients.size();
	totalIterationCount = 0;
}

bool MasterPlanner9::isAnyClientSearching() const {
	for (ClientsMap::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it.value().bestsMinCost != Planner9::InfiniteCost) {
			return true;
		}
	}
	return false;
}

void MasterPlanner9::sendGetNode(ChunkedDevice* device) {
	stream.setDevice(device);
	stream.write(CMD_GET_NODE);
	device->flush();
}

void MasterPlanner9::sendScope(ChunkedDevice* device) {
	stream.setDevice(device);
	stream.write(CMD_PROBLEM_SCOPE);
	stream.write(problem.scope);
	device->flush();
}

void MasterPlanner9::sendInitialNode(ChunkedDevice* device) {
	sendNode(device, *initialNode);
}

void MasterPlanner9::sendNode(ChunkedDevice* device, const SimplePlanner9::SearchNode& node) {
	stream.setDevice(device);
	stream.write(CMD_PUSH_NODE);
	stream.write<quint32>(1);
	stream.write(node);
	device->flush();
}

void MasterPlanner9::sendStop(ChunkedDevice* device) {
	stream.setDevice(device);
	stream.write(CMD_STOP);
	device->flush();
}

