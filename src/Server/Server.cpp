#include "Server.hpp"

Server:: Server(const Config& config): serverSocket(-1), epollFd(-1), serverConfig(config) {
	std::cout << "Server: Initializing...\n";
	struct addrinfo* result = setupAddressInfo();
	bindToAddress(result);
	freeaddrinfo(result);
	setNonBlocking(serverSocket);
	listenForConnections();
	std::cout << "Server: Initialized successfully\n";
}

Server::~Server() {
	cleanup();
	std::cout << "Server: Cleaned up resources\n";
}

void Server::start() {
	std::cout << "Server: Starting...\n";
	createEpoll();
	addSocketToEpoll(serverSocket);
	eventLoop();
}

void Server::cleanup() {
	if (serverSocket != -1) {
		close(serverSocket);
		serverSocket = -1;
	}
	if (epollFd != -1) {
		close(epollFd);
		epollFd = -1;
	}
}

struct addrinfo* Server::setupAddressInfo() {
	struct addrinfo hints, *result;

	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	// Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP SOCKET
	hints.ai_flags = AI_PASSIVE;	// Use wildcard IP address

	int status = getaddrinfo(serverConfig.host.c_str(), serverConfig.port.c_str(), &hints, &result);
	if (status != 0) {
		std::cerr << "Error in getaddrinfo: " << gai_strerror(status) << "\n";
		cleanup();
	}

	return result;
}

void Server::bindToAddress(struct addrinfo* result) {
	struct addrinfo* rp;

	// Attempt to bind to one of the addresses obtained from getaddrinfo
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		serverSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (serverSocket == -1) continue;

		int reuse = 1;
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
			std::cerr << "Error setting SO_REUSEADDR: " << strerror(errno) << "\n";
			cleanup();
			return;
		}

		if (bind(serverSocket, rp->ai_addr, rp->ai_addrlen) == 0)
			break; // Successfully bound to an address

		close(serverSocket);
	}

	if (rp == NULL) {
		std::cerr << "Could not bind to any address\n";
		cleanup();
		return;
	}
}


void Server::listenForConnections() {
	if (listen(serverSocket, SOMAXCONN) == -1) {
		std::cerr << "Error listening on socket\n";
		cleanup();
	}
}

int Server::setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Error setting non-blocking flag on socket\n";
		return -1;
	}
	return 0;
}

void Server::createEpoll() {
	epollFd = epoll_create1(0);
	if (epollFd == -1) {
		std::cerr << "Error creating epoll instance\n";
		cleanup();
	}
}

void Server::addSocketToEpoll(int fd) {
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLOUT; // Enable edge-triggered mode
	event.data.fd = fd;

	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
		std::cerr << "Error adding socket to epoll\n";
		cleanup();
	}
}

void Server::eventLoop() {
	const int maxEvents = 64;
	struct epoll_event events[maxEvents];

	while (true) {
		int numEvents = epoll_wait(epollFd, events, maxEvents, -1);
		if (numEvents == -1) {
			std::cerr << "Error in epoll_wait: " << strerror(errno) << "\n";
			break;
		}

		handleEvents(events, numEvents);
	}
}

void Server::handleEvents(struct epoll_event* events, int numEvents) {
	for (int i = 0; i < numEvents; ++i) {
		int eventFd = events[i].data.fd;
		if (eventFd == serverSocket) {
			acceptConnections();
		} else {
			handleExistingConnection(eventFd);
		}
	}
}

void Server::handleExistingConnection(int eventFd) {
	std::map<int, ClientHandler>::iterator it = clientsZone.find(eventFd);
	if (it == clientsZone.end()) {
		// New client
		std::pair<std::map<int, ClientHandler>::iterator, bool> ret;
		ret = clientsZone.insert(std::pair<int, ClientHandler>(eventFd, ClientHandler(eventFd, epollFd)));
		it = ret.first;
	}

	ClientHandler& client = it->second;
	if (client.done) {
		epoll_ctl(epollFd, EPOLL_CTL_DEL, eventFd, NULL);
		close(eventFd);
		clientsZone.erase(it);
	} else {
		client.refresh();
	}
}

void Server::acceptConnections() {
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
	if (clientSocket == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			std::cerr << "Error accepting connection: " << strerror(errno) << "\n";
		}
		return;
	}
	std::cout << "Server: Accepted new connection\n";

	if (setNonBlocking(clientSocket) == -1) {
		std::cerr << "Error setting non-blocking flag on client socket\n";
		close(clientSocket);
		return;
	}

	addSocketToEpoll(clientSocket);
}

