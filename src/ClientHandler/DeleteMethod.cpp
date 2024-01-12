#include "ClientHandler.hpp"

void ClientHandler::DeleteMethod(){
    std::string command = "rm -rf " + fullLocation;

    if(std::system(command.c_str()))
        throw HttpError(InternalServerError, "Internal Server Error");
    setResponseParams("200", "OK", "", "error_pages/deleted.html");
}