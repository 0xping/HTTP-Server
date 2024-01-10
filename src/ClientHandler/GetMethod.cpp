#include "ClientHandler.hpp"

void ClientHandler::GetMethod(){
    if (isDir)
        std::cout << std::endl;// process index & autoindex
    else{
        if (isCGIfile)
            std::cout << std::endl;// call cgi
        else
            setResponseParams("200", "OK", "", fullLocation); // pass file to send
    }
}