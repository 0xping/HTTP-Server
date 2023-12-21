#include "src/Server/Server.hpp"
#include "src/Config/Config.hpp"

int main() {
	// Sample Config
	Config serverConfig;
	serverConfig.port = "8080";
	serverConfig.host = "10.13.9.3";  // IP address

	// Create and start the server
	Server myServer(serverConfig);
	myServer.start();
	return 0;
}
