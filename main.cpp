#include "src/Server/Server.hpp"
#include "src/Config/Config.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/epoll.h>
#include "utils/utils.hpp"
#include "src/Cluster/Cluster.hpp"

int main() {

	std::vector<Config> configs;
	Config config("127.0.0.1", "8080"), config1("127.0.0.1", "8082");
	configs.push_back(config);
	configs.push_back(config1);

	Cluster Servers(configs);

	return 0;
}