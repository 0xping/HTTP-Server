#include "src/Server/Server.hpp"
#include "src/Config/Config.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/epoll.h>
#include "utils/utils.hpp"

int main() {
	Config config("127.0.0.1", "8080");
	Server server(config);
	server.start();


	return 0;
}

