#pragma once

#include "../webserv.hpp"


typedef struct s_location {
    std::string              path;
    std::string              root;
    std::string              _return;
    std::string              upload_path;
    std::vector<std::string> index;
    std::vector<std::string> allow_methods;    
    std::map<std::string, std::string> cgi_path;
    bool                     autoindex;
} t_location;

typedef struct s_server {
    int                                 port;    
    std::string                         ip;
    std::string                         server_name;
    std::string                         max_body_size;
    std::vector<t_location>             locations;
    std::map<std::string, std::string>  error_pages;
} t_server;


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
    public:
        Config(std::string config_file);
        std::vector<t_server> servers;
};