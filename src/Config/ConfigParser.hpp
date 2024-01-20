#pragma once

// #include "../../webserv.hpp"
#include "ServerConfig.hpp"
#include "Location.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <map>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <cctype>
#include <unistd.h>

class ConfigParser{
    void parse_config(std::string config_file);
    void parse_server(std::ifstream& file);
    void parse_line(std::string& line, std::ifstream& file, ServerConfig& server);
    void check_leading_spaces(std::string& str, int n);
    void checkTrailingSpaces(std::string& directive, std::string &line);
    void parse_ip(std::string& str, std::string& line, ServerConfig& server);
    void parse_in_line(std::string &str, std::string& line, std::string& directive);
    void parse_error_pages(std::string& line, std::string _2ndfield, std::ifstream& file, ServerConfig& server);
    void parse_location(std::string& line, std::ifstream& file, ServerConfig& server);
    void parse_index(std::string& line, std::ifstream& file, Location& location);
    void parse_methods(std::string* str, std::ifstream& file, Location& location);
    void parse_cgi(std::string *str, std::ifstream& file, Location& location);
    void parse_body_size(std::string& str, std::string& line, ServerConfig& server);
    void check_server(ServerConfig& server);
    void check_location(Location& location, ServerConfig& server);
    public:
        ConfigParser(std::string config_file);
        ConfigParser(){};
        ServerConfig& getServerConfig(std::string& ip, int port, std::string& host);
        std::vector<ServerConfig> servers;
};


typedef ConfigParser ClusterConfig;
