#pragma once

#include "../../webserv.hpp"
#include "T_srver.hpp"
#include "T_location.hpp"

class Config{         
    void parse_config(std::string config_file);
    void parse_server(std::ifstream& file);
    void parse_line(std::string& line, std::ifstream& file, t_server& server);
    void check_leading_spaces(std::string& str, int n);
    void checkTrailingSpaces(std::string& directive, std::string &line);
    void parse_ip(std::string& str, std::string& line, t_server& server);
    void parse_in_line(std::string &str, std::string& line, std::string& directive);
    void parse_error_pages(std::string& line, std::string _2ndfield, std::ifstream& file, t_server& server);
    void parse_location(std::string& line, std::ifstream& file, t_server& server);
    void parse_index(std::string& line, std::ifstream& file, t_location& location);
    void parse_methods(std::string* str, std::ifstream& file, t_location& location);
    void parse_cgi(std::string *str, std::ifstream& file, t_location& location);
    void parse_body_size(std::string& str, std::string& line, t_server& server);
    void check_server(t_server& server);
    void check_location(t_location& location, t_server& server);
    public:
        Config(std::string config_file);
        std::vector<t_server> servers;
};