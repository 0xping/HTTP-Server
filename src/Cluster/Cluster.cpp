#include "Cluster.hpp"

Cluster::Cluster(const ClusterConfig &configs)
{
	this->config = configs;
	createEpoll();

	std::vector <ServerConfig> serverConfigs = configs.servers;
	for (size_t i = 0; i < serverConfigs.size() ; i++)
	{
		// passing all the cluster config the each server
		Server *serv = new Server(serverConfigs[i], configs);
		if (serv->serverSocket != -1) servers.push_back(serv);
		else
			delete serv;
	}
	for (size_t i = 0; i < servers.size() ; i++)
		addSocketToEpoll(servers[i]->serverSocket);
	eventLoop();
}


void Cluster::createEpoll() {
	epollFd = epoll_create(64);
	if (epollFd == -1) {
		std::cerr << "Error creating epoll instance\n";
		cleanup();
	}
}


void Cluster::cleanup()
{
	for (size_t i = 0; i < servers.size() ; i++)
		servers[i]->cleanup();

	if (epollFd != -1) {
		close(epollFd);
		epollFd = -1;
	}
}

void Cluster::addSocketToEpoll(int fd) {
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = fd;

	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
		std::cerr << "Error adding socket to epoll\n";
		cleanup();
	}
}


int Cluster::setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Error setting socket to None-Blocking mode\n";
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Error setting socket to None-Blocking mode\n";
		return -1;
	}
	return 0;
}


Server& Cluster::getServerByClientFd(int fd)
{
	for (size_t i = 0; i < servers.size(); i++) {
		if (servers[i]->connectedClients.find(fd) != servers[i]->connectedClients.end())
			return *servers[i];
	}

	throw std::runtime_error("getServerByClientFd: Server not found for given file descriptor");
}

Server& Cluster::getServerByFd(int fd)
{
	for (size_t i = 0; i < servers.size(); i++) {
		if (servers[i]->serverSocket == fd) {
			return *servers[i];
		}
	}

	throw std::runtime_error("getServerByFd:Server not found for given file descriptor");
}

bool Cluster::isServerFd(int fd)
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		if (servers[i]->serverSocket == fd) {
			return true;
		}
	}
	return false;
}

void Cluster::eventLoop() {
	const int maxEvents = 64;
	struct epoll_event events[maxEvents];

	while (true) {
		//std::cout << clientsZone.size() << " ---\n\n";
		std::cout << "waiting..." << std::endl;
		int numEvents = epoll_wait(epollFd, events, maxEvents, -1);
		if (numEvents == -1) {
			std::cerr << "Error in epoll_wait: " << strerror(errno) << "\n";
			break;
		}

		handleEvents(events, numEvents);
	}
}

void Cluster::handleEvents(struct epoll_event* events, int numEvents) {
	for (int i = 0; i < numEvents; ++i) {
		int eventFd = events[i].data.fd;
		if (isServerFd(eventFd)) {
			acceptConnections(eventFd);
		} else {
			handleExistingConnection(eventFd, events[i].events);
		}
	}
}


void Cluster::handleExistingConnection(int eventFd, uint32_t eventsData) {
	ServerConfig &serverConfig = getServerByClientFd(eventFd).serverConfig;
	std::map<int, ClientHandler>::iterator it = clientsZone.find(eventFd);
	if (it == clientsZone.end()) {
		// New client
		std::pair<std::map<int, ClientHandler>::iterator, bool> ret;
		ret = clientsZone.insert(std::pair<int, ClientHandler>(eventFd, ClientHandler(eventFd, epollFd, serverConfig, config)));
		it = ret.first;
	}

	ClientHandler& client = it->second;

	if ((eventsData & EPOLLIN) && client.status == Receiving)
	{
		std::cout << "ClientHandler: Client Receiving..." << std::endl;
		client.readyToReceive();
	}
	else if ((eventsData & EPOLLOUT) && client.status == Sending)
	{
		std::cout << "ClientHandler: Client Sending..." << std::endl;
		client.readyToSend();
	}

	if (client.status == Closed)
	{
		// ...
		std::cout << "connection Closed Remove client from the Map" << std::endl;
		epoll_ctl(epollFd, EPOLL_CTL_DEL, client.clientFd, NULL);
		struct epoll_event events;
		epoll_wait(epollFd, &events, 1, -1);
		close(client.clientFd);
		clientsZone.erase(it);
	}
}

void Cluster::acceptConnections(int serverSocket) {
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
	if (clientSocket == -1) {
		std::cerr << "Error accepting connection: " << strerror(errno) << "\n";
		return;
	}
	std::cout << "Server: Accepted new connection\n";

	if (setNonBlocking(clientSocket) == -1) {
		std::cerr << "Error setting non-blocking flag on client socket\n";
		close(clientSocket);
		return;
	}

	addSocketToEpoll(clientSocket);
	getServerByFd(serverSocket).connectedClients.insert(clientSocket);
}
