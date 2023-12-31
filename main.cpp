#include "src/Server/Server.hpp"
#include "src/Config/ConfigParser.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/epoll.h>
#include "utils/utils.hpp"
#include "src/Cluster/Cluster.hpp"

int main(int ac, char const *av[])
{
	std::string configFile = "config.yaml";
	if(ac < 2)
		std::cerr << "no config file provided using ./config.yaml as default\n\n";
	else
		configFile = av[1];

	ConfigParser clusterConfig(configFile);
	Cluster servers(clusterConfig);
	return 0;
}

