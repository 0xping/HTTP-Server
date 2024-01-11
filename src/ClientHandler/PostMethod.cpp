#include "ClientHandler.hpp"

// bool

// void ClientHandler::chunked_handler()
// {
//     if (!this->tmpFiles.size())
//     {
//         if (this->isDir)
//         {
//             if (message.headers.find("Content-Type") != message.headers.end())
//                 this->tmpFiles.push_back(generateUniqueFileName(this->fullLocation, getExtensionPost(message.headers["Content-Type"])));
//             else
//                 this->tmpFiles.push_back(generateUniqueFileName(this->fullLocation, ".tmp"));
//         }
//         else
//             exit(404);
//     }
//     if (!counter)
//     {
//         if (readingBuffer.find("\r\n") == std::string::npos)
//         {
//             // need to be handled: wait? 
//             exit(500);
//         }
//         if (!isValidHex(readingBuffer.substr(0, readingBuffer.find("\r\n")).toStr(), counter))
//             exit(404);
//         this->readingBuffer = this->readingBuffer.substr(readingBuffer.find("\r\n") + 2, readingBuffer.size());
//     }
//     if (this->readingBuffer.size() >= counter)
//         return;
// }

void ClientHandler::regular_data_handler()
{
    if (!this->tmpFiles.size())
    {
        
        if (this->isDir)
        {
            if (message.headers.find("Content-Type") != message.headers.end())
                this->tmpFiles.push_back(generateUniqueFileName(this->fullLocation, getExtensionPost(message.headers["Content-Type"])));
            else
                this->tmpFiles.push_back(generateUniqueFileName(this->fullLocation, ".tmp"));
        }
        else if (isCGI)
            exit(404);
        else
            throw HttpError(MethodNotAllowed, "Method Not Allowed");
    }
    // std::ofstream postedFile(this->tmpFiles[0], std::ios::app);
    // if (!postedFile)
    //     throw HttpError(InternalServerError, "Internal Server Error");
    
}

void ClientHandler::PostMethod()
{
    readFromSocket();


    // if (message.headers.find("Transfer-Encoding") != message.headers.end())
    // {
    //     // implement chunked
    //     chunked_handler();
    // }
    // else if (message.headers.find("Transfer-Encoding") == message.headers.end())
    // {
        
    // }
	// std::map<std::string, std::string>::iterator contentLength = message.headers.find("Content-Length");
    if (message.headers.find("Content-Length") == message.headers.end())
    {
        throw HttpError(BadRequest, "Bad Request");
    }
    else
    {
        if (message.headers.find("Content-Type") != message.headers.end())
        {
            // store it by mime type or .tmp
        }
        else
        {
            // read and ignore
        }
    }
}