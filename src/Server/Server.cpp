#include "Server.hpp"

Server:: Server(const Config& config): serverSocket(-1), epollFd(-1), serverConfig(config) {
	std::cout << "Server: Initializing...\n";
	initAndBindSocket();
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

void Server::initAndBindSocket() {
	struct addrinfo hints, *result, *rp;

	// Set up hints structure
	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	// Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP SOCKET
	hints.ai_flags = AI_PASSIVE;	// Use wildcard IP address

	// Use getaddrinfo to obtain address information
	// int status = getaddrinfo("10.13.9.3", "8080", &hints, &result);
	int status = getaddrinfo(serverConfig.host.c_str(), serverConfig.port.c_str(), &hints, &result);
	if (status != 0) {
		std::cerr << "Error in getaddrinfo: " << gai_strerror(status) << "\n";
		cleanup();
		return;
	}

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

	freeaddrinfo(result); // Free the address information obtained from getaddrinfo
	if (rp == NULL) {
		std::cerr << "Could not bind to any address\n";
		cleanup();
		return;
	}

	// Set server socket to non-blocking mode
	if (setNonBlocking(serverSocket) == -1) {
		std::cerr << "Error setting non-blocking flag on socket\n";
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
		std::cerr << "Error setting non-blocking flag\n";
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
	event.events = EPOLLIN | EPOLLET; // Enable edge-triggered mode
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

		for (int i = 0; i < numEvents; ++i) {
			if (events[i].data.fd == serverSocket) {
				// New connection
				acceptConnections();
			} else {
				// Existing connection
				handleConnection(events[i].data.fd);
			}
		}
	}
}

void Server::acceptConnections() {
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	while (true) {
		errno = 0; // Reset errno
		int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
		if (clientSocket == -1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				std::cerr << "Error accepting connection: " << strerror(errno) << "\n";
			} // or No more incoming connections
			break;
		}

		std::cout << "Server: Accepted new connection\n";

		// Set client socket to non-blocking mode
		if (setNonBlocking(clientSocket) == -1) {
			std::cerr << "Error setting non-blocking flag on client socket\n";
			close(clientSocket);
			continue;
		}

		// Add client socket to epoll
		addSocketToEpoll(clientSocket);
	}
}

void Server::handleConnection(int clientSocket) {
	std::cout << "Server: Handling connection\n";
	const int bufferSize = 4096;
	char buffer[bufferSize];
	std::string requestData;

	while (true) { // Receive data from the client
		ssize_t bytesRead = recv(clientSocket, buffer, bufferSize - 1, 0);
		if (bytesRead == -1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				std::cerr << "Error receiving data: " << strerror(errno) << "\n";
				close(clientSocket);
				return;
			} // or No more data to read
			break;
		}

		if (bytesRead == 0) {
			std::cout << "Connection closed by client\n";
			break;
		}

		// Null-terminate the received data
		buffer[bytesRead] = '\0';

		// Print the received HTTP request
		std::cout << "Received HTTP request:\n" << buffer << "\n";

		// parse the request and send the respond;
		// std::string response = RequestHandler(response);
		// send(clientSocket, response.c_str(), response.size(), 0);
	}

	// This is just a demo response, parsing the request and sending the response should be done inside the loop
	std::string response =
		"HTTP/1.1\r\n"
		"Date: Sun, 18 Oct 2012 10:36:20 GMT\r\n"
		"Server: Apache/2.2.14 (Win32)\r\n"
		"Content-Length: 230\r\n"
		"Connection: Closed\r\n"
		"Content-Type: text/html; charset=iso-8859-1\r\n"
		"\r\n"
		"<h1>Hello, World!<h1>";


	send(clientSocket, response.c_str(), response.size(), 0);

	// After handling the connection, remove the client socket from epoll
	epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, NULL);
	close(clientSocket);
	std::cout << "Server: Connection handled\n";
}
