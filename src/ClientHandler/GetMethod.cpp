#include "ClientHandler.hpp"

void ClientHandler::GetMethod(){
    if (isDir)
    {
        if (!location.index.empty()){
            if (GetIndex())
                return ;
        }
        if (!location.autoindex)
            throw HttpError(Forbidden, "Forbidden");
        else
            GetAutoIndex();
    }
    else{
        if (isCGIfile)
            execCGI();
        else
            setResponseParams("200", "OK", "", fullLocation); // pass file to send
    }
}

int ClientHandler::GetIndex(){
    std::vector<std::string>::iterator it = location.index.begin();
    for (;it != location.index.end(); it++)
    {
        if (!access(it->c_str(), R_OK)){
            setResponseParams("200", "OK", "", *it);
            return 1;
        }
    }
    return 0;
}

void ClientHandler::GetAutoIndex(){
    char nameBuffer[L_tmpnam] = {0};
    std::string AIfile = std::tmpnam(nameBuffer) + std::string(".html");
    std::ofstream autoindexFile(AIfile.c_str());

    tmpFiles.push_back(AIfile);
    if (!autoindexFile.is_open())
        throw HttpError(InternalServerError, "Internal Server Error GET1");
    // protect
    
    DIR *dir = opendir(fullLocation.c_str());

    if (dir == NULL){
        autoindexFile.close();
        throw HttpError(InternalServerError, "Internal Server Error GET2");
    }

    // html header
    autoindexFile << "<html>\n<head>\n<title>Autoindex</title>\n</head>\n<body>\n";
    autoindexFile << "<h1>Autoindex</h1>\n<ul>\n";

    // read directory
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL){
        if (entry->d_name[0] != '.'){
            // write a list item with a link to the file
            autoindexFile << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>\n";
        }
    }

    // close html file
    autoindexFile << "</ul>\n</body>\n</html>";
    autoindexFile.close();
    setResponseParams("200", "OK", "", AIfile);
    closedir(dir);
}