#include "ConfigParser.hpp"

ConfigParser::ConfigParser(std::string config_file){
    parse_config(config_file);
}

void ConfigParser::parse_config(std::string config_file){
    int len;
    std::ifstream file;

    len = config_file.length();
    if (len >= 5 && config_file[0] != '.' && (!strcmp(&config_file[len - 5], ".yaml") || !strcmp(&config_file[len - 4], ".yml")))
    {
        file.open(config_file.c_str());
        if (!file.is_open())
            cerrAndExit("Error: can't open config file!", 1);
        parse_server(file);
    }
    else
        cerrAndExit("Invalid file extention!", 1);
}

void ConfigParser::parse_server(std::ifstream& file){
    std::string tmp;
    int s = 0;
    ServerConfig server;

    server.max_body_size = 1048576;
    while(std::getline(file, tmp)){
        if (isAllSpaces(tmp))
            continue;
        trim_spaces(tmp, 0);
        if (!s && tmp != "server:")
            cerrAndExit("ERROR: file must start with server!", 1);
        if (!s && tmp == "server:")
            s = 1;
        else if (s && tmp == "server:"){
            check_server(server);
            servers.push_back(server);
            file.seekg(-tmp.length()-1, std::ios_base::cur);
            parse_server(file);
            return ;
        }
        else
            parse_line(tmp, file, server);
    }
    check_server(server);
    servers.push_back(server);
}

void ConfigParser::check_server(ServerConfig& server){
    if (server.ip.empty() || server.locations.empty())
        cerrAndExit("ERROR: there has to be at least one server! every server has to have a listen field and at least 1 location!", 1);
    if (server.locations.find("/") == server.locations.end())
        cerrAndExit("ERROR: default location '/' is mandatory!", 1);
}

void ConfigParser::check_leading_spaces(std::string& str, int n){
    int i = 0;
    while (str[i] && std::isspace(str[i]))
        i++;
    if (i != n)
        cerrAndExit("ERROR:\nInconsistent use of spaces -> " + str, 1);
}

void ConfigParser::checkTrailingSpaces(std::string& directive, std::string &line){
    if (directive[directive.length() - 1] == ' ')
        cerrAndExit("ERROR:\nInvalid syntax -> " + line + " - directives can't be followed by spaces!", 1);
}

void ConfigParser::parse_ip(std::string& ip, std::string& line, ServerConfig& server){
    int i = 0;
    std::string *ip_field;
    std::string *splited = split(ip, ':');
    std::string tmp;

    if (!server.ip.empty())
        cerrAndExit("ERROR: Found duplicate listen directive!", 1);
    if (ip.empty())
        cerrAndExit("ERROR:\n-> " + line + " - Field can't be empty!", 1);
    if (splited[1].empty())
        cerrAndExit("ERROR:\n-> " + ip + " - Invalid listen field!", 1);
    ip_field = split(splited[0], '.');
    while (i < 4){
        if ((i == 3 && (!ip_field[1].empty() || tmp.find('.') != std::string::npos)) || ip_field[0].empty())
            cerrAndExit("ERROR:\nInvalid ip! -> " + ip, 1);
        if (!isAllDigit(ip_field[0]) || ip_field[0].length() > 3 || std::atoi(ip_field[0].c_str()) > 255)
            cerrAndExit("ERROR:\nInvalid ip! -> " + ip, 1);
        tmp = ip_field[1];
        delete[] ip_field;
        ip_field = split(tmp, '.');
        i++;
    }
    if (!isAllDigit(splited[1]))
        cerrAndExit("ERROR:\nInvalid port! -> " + ip, 1);
    server.port = std::atoi(splited[1].c_str());
    server.ip = splited[0];
    if (splited[1].length() > 5 || server.port < 1024 || server.port > 65535)
        cerrAndExit("ERROR:\nInvalid port! -> " + ip, 1);
    delete[] splited;
    delete[] ip_field;
}

void ConfigParser::parse_in_line(std::string& str, std::string& line, std::string& directive){
    if (str.empty())
        cerrAndExit("ERROR:\nField can't be empty! -> " + line, 1);
    if (!directive.empty())
        cerrAndExit("ERROR: Found duplicate directive!\n-> " + line, 1);
    if (str.find(" ") != std::string::npos)
        cerrAndExit("ERROR:\nField can't have spaces! -> " + line, 1);
    directive = str;
}

void ConfigParser::parse_line(std::string& line, std::ifstream& file, ServerConfig& server){
    std::string *splited;

    splited = split(line, ':');
    check_leading_spaces(line, 2);
    checkTrailingSpaces(splited[0], line);
    trim_spaces(splited[0], 1);
    trim_spaces(splited[1], 1);
    if (splited[0] == "listen")
        parse_ip(splited[1], line, server);
    else if (splited[0] == "max_client_body_size")
        parse_body_size(splited[1], line, server);
    else if (splited[0] == "server_name")
        parse_in_line(splited[1], line, server.server_name);
    else if (splited[0] == "error_page")
        parse_error_pages(line, splited[1], file, server);
    else if (splited[0] == "location"){
        if (!server.locations.empty())
            cerrAndExit("ERROR: Found duplicate location directive!", 1);
        if (!isAllSpaces(splited[1]) && !splited[1].empty())
            cerrAndExit("ERROR: Invalid syntax -> " + line, 1);
        while(std::getline(file, line)){
            if (isAllSpaces(line))
                continue;
            break ;
        }
        parse_location(line, file, server);
    }
    else
        cerrAndExit("ERROR: Invalid directive found -> " + splited[0], 1);
    delete[] splited;
}

void ConfigParser::parse_body_size(std::string& str, std::string& line, ServerConfig& server){
    if (!server.body_size.empty())
        cerrAndExit("ERROR: Found duplicate max_client_body_size directive!", 1);
    parse_in_line(str, line, server.body_size);
    if (!isAllDigit(server.body_size))
        cerrAndExit("ERROR: Invalid max body size!", 1);
    std::stringstream size(server.body_size);
    size >> server.max_body_size;
    if (size.fail())
        cerrAndExit("ERROR: possible overflow of body size -> " + line, 1);
}

void ConfigParser::parse_methods(std::string* str, std::ifstream& file, Location& location){
    std::string line;

    if (!str[1].empty() && !isAllSpaces(str[1]))
        cerrAndExit("ERROR: allowed methods should be provided in a list!", 1);
    while (std::getline(file, line)){
        int i = 0;
        if (isAllSpaces(line))
            continue;
        while (std::isspace(line[i]))
            i++;
        if (i < 8 && !(i % 2)){
            file.seekg(-line.length()-1, std::ios_base::cur);
            break;
        }
        check_leading_spaces(line, 8);
        trim_spaces(line, 1);
        if (strncmp(line.c_str(), "- ", 2))
            cerrAndExit("ERROR: list elements must start with '- '!\n-> " + line, 1);
        std::string method(&line[2]);
        if (method != "GET" && method != "POST" && method != "DELETE")
            cerrAndExit("ERROR: found invalid method!\n-> " + method, 1);
        if (std::find(location.allow_methods.begin(), location.allow_methods.end(), method) != location.allow_methods.end())
            cerrAndExit("ERROR: found duplicate methods in " + location.path, 1);
        location.allow_methods.push_back(method);
    }
}

void ConfigParser::parse_location(std::string& line, std::ifstream& file, ServerConfig& server){
    std::string *splited;
    Location location;

    check_leading_spaces(line, 4);
    trim_spaces(line, 1);
    if (line[0] != '/')
        cerrAndExit("ERROR: invalid syntax: locations have to start with '/'\n-> " + line, 1);
    splited = split(line, ':');
    checkTrailingSpaces(splited[0], line);
    if (!splited[1].empty())
        cerrAndExit("ERROR: Invalid syntax -> " + line, 1);
    location.path = splited[0];
    delete[] splited;
    while (std::getline(file, line)){
        int i = 0;
        if (isAllSpaces(line))
            continue;
        while(std::isspace(line[i]))
            i++;
        if (i == 0){
            file.seekg(-line.length()-1, std::ios_base::cur);
            break;
        }
        if (i == 4){
            check_location(location, server);
            server.locations[location.path]=location;
            parse_location(line, file, server);
            return ;
        }
        check_leading_spaces(line, 6);
        trim_spaces(line, 1);
        splited = split(line, ':');
        checkTrailingSpaces(splited[0], line);
        trim_spaces(splited[1], 1);
        if (splited[0] == "root")
            parse_in_line(splited[1], line, location.root);
        else if (splited[0] == "autoindex" || splited[0] == "index")
            parse_index(line, file, location);
        else if (splited[0] == "allow_methods")
            parse_methods(splited, file, location);
        else if (splited[0] == "cgi_path")
            parse_cgi(splited, file, location);
        else if (splited[0] == "return")
            parse_in_line(splited[1], line, location._return);
        else if (splited[0] == "upload_path")
            parse_in_line(splited[1], line, location.upload_path);
        else
            cerrAndExit("ERROR: Invalid directive found in locations!", 1);
        delete[] splited;
    }
    check_location(location, server);
    server.locations[location.path]= location;
}

void ConfigParser::check_location(Location& location, ServerConfig& server){
    if (location.root.empty())
        cerrAndExit("ERROR: root wasn't provided for " + location.path + "!", 1);
    if (server.locations.find(location.path) != server.locations.end())
        cerrAndExit("ERROR: found duplicate path in locations!\n-> " + location.path, 1);
}

void ConfigParser::parse_cgi(std::string* str, std::ifstream& file, Location& location){
    std::string line;
    std::string *splited;

    if (!isAllSpaces(str[1]) && !str[1].empty())
        cerrAndExit("ERROR: cgi paths have to be a list!", 1);
    while (std::getline(file, line)){
        int i = 0;
        if (isAllSpaces(line))
            continue;
        while (std::isspace(line[i]))
            i++;
        if (i < 8 && !(i % 2)){
            file.seekg(-line.length()-1, std::ios_base::cur);
            break;
        }
        check_leading_spaces(line, 8);
        trim_spaces(line, 1);
        if (strncmp(line.c_str(), "- ", 2))
            cerrAndExit("ERROR: list elements have to start with '- '!", 1);
        splited = split(&line[2], ':');
        checkTrailingSpaces(splited[1], line);
        trim_spaces(splited[1], 1);
        if (location.cgi_path.find(splited[0]) != location.cgi_path.end())
            cerrAndExit("ERROR: found duplicate cgi path " + splited[0] + "in " + location.path, 1);
        location.cgi_path[splited[0]] = splited[1];

        delete[] splited;
    }
}

void ConfigParser::parse_index(std::string& line, std::ifstream& file, Location& location){
    std::string* splited;

    splited = split(line, ':');
    trim_spaces(splited[1], 1);
    if (splited[0] == "autoindex" && (splited[1] == "off" || splited[1] == "on"))
        location.autoindex = (splited[1].length() == 2);
    else if (splited[0] == "autoindex")
        cerrAndExit("ERROR: autoindex can only be 'on' or 'off'!", 1);
    else if (splited[0] == "index" && !splited[1].empty())
        cerrAndExit("ERROR: index directive takes a list!\n-> " + line, 1);
    else if (splited[0] == "index"){
        while (std::getline(file, line)){
            int i = 0;
            if (isAllSpaces(line))
                continue;
            while (std::isspace(line[i]))
                i++;
            if (i < 8 && !(i % 2)){
                file.seekg(-line.length()-1, std::ios_base::cur);
                break;
            }
            check_leading_spaces(line, 8);
            trim_spaces(line, 1);
            if (strncmp("- ", line.c_str(), 2))
                cerrAndExit("ERROR: list elements have to start with '- '!", 1);
            std::string index(&line[2]);
            if (std::find(location.index.begin(), location.index.end(), index) != location.index.end())
                cerrAndExit("ERROR: found duplicate index in " + location.path, 1);
            location.index.push_back(std::string(&line[2]));
        }
    }
    delete[] splited;
}

void ConfigParser::parse_error_pages(std::string& line, std::string _2ndfield, std::ifstream& file, ServerConfig&  server){
    std::string *splitted;
    std::string tmp;

    if (!_2ndfield.empty())
        cerrAndExit("ERROR:\nField can't be inline! -> "  + line, 1);
    while (std::getline(file, tmp)){
        int i = 0;
        if (isAllSpaces(tmp))
            continue;
        while (tmp[i] && std::isspace(tmp[i]))
            i++;
        if (i == 2){
            file.seekg(-tmp.length()-1, std::ios_base::cur);
            break ;
        }
        check_leading_spaces(tmp, 4);
        trim_spaces(tmp, 1);
        if (strncmp(tmp.c_str(), "- ", 2))
            cerrAndExit("ERROR: Invalid syntax.\n-> list elements have to start with '- ' -> " + tmp, 1);
        splitted = split(&tmp[1], ':');
        trim_spaces(splitted[0], 1);
        trim_spaces(splitted[1], 1);
        if (server.error_pages.find(splitted[0]) != server.error_pages.end())
            cerrAndExit("ERROR: found duplicate error page: -> " + tmp, 1);
        server.error_pages[splitted[0]] = splitted[1];
        delete[] splitted;
    }
}


ServerConfig& ConfigParser::getServerConfig(std::string& ip, int port, std::string& host){
    std::vector<ServerConfig*> tmp;

    for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); it++){                
        if (ip == it->ip && port == it->port){
            if (host == it->server_name)
                return *it;
            if (tmp.empty())
                tmp.push_back(&(*it));
        }
    }
    return *tmp[0];
}
