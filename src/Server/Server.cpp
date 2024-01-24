#include "Server.hpp"

Server::Server(const ServerConfig& serverConfig, const ClusterConfig& clusterConfig):
	serverSocket(-1),
	serverConfig(serverConfig),
	clusterConfig(clusterConfig)
{
	std::cout << "Server: Initializing...\n";
	struct addrinfo* result = setupAddressInfo();
	bindToAddress(result);
	freeaddrinfo(result);
	listenForConnections();
	std::cout << "Server: Initialized successfully\n";
}

Server::~Server() {
	cleanup();
	std::cout << "Server: Cleaned up resources\n";
}

struct addrinfo* Server::setupAddressInfo() {
	struct addrinfo hints, *result;

	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	// Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP SOCKET
	hints.ai_flags = AI_PASSIVE;	// Use wildcard IP address

	std::string portAsStr;
	std::stringstream ss;
	ss << serverConfig.port;
	ss >> portAsStr;

	std::cout << serverConfig.ip << ":" << portAsStr << "\n\n";

	int status = getaddrinfo(serverConfig.ip.c_str(), portAsStr.c_str(), &hints, &result);
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

void Server::cleanup() {
	if (serverSocket != -1) {
		close(serverSocket);
		serverSocket = -1;
	}
}