#include "ClientHandler.hpp"

void writeToFile(const std::string &data, const std::string& fileName)
{
    if (!freopen(fileName.c_str(), "a", stdout))
        throw HttpError(InternalServerError, "Internal Server Error");
    std::cout << data;
    if (!freopen("/dev/tty", "a", stdout))
        throw HttpError(InternalServerError, "Internal Server Error");
}  

void ClientHandler::mutiple_part_handler()
{

}

void ClientHandler::chunked_handler()
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
        else if (!isCGIfile)
            throw HttpError(Forbidden, "Forbidden");
        else
            // handle cgi
            // isCGIfile = isCGIfile;
            std::cout << "handle cgi for post!" << std::endl;
    }
    if (!chunkSize)
    {
        if (readingBuffer.find("\r\n") != std::string::npos)
        {
            if (!isValidBase(readingBuffer.substr(0, readingBuffer.find("\r\n")).toStr(), chunkSize, 16))
                throw HttpError(BadRequest, "Bad Request");
            if (!chunkSize)
            {
                if (readingBuffer.find("\r\n\r\n") != std::string::npos)
                    this->status = Sending;
                else
                    return;
            }
            else
            {
                readingBuffer = readingBuffer.substr(0, readingBuffer.find("\r\n") + 2);
                counter += readingBuffer.find("\r\n") + 2;
            }
        }
        else
            return;
    }
    if (this->readingBuffer.size() >= chunkSize)
        return;
    if (this->readingBuffer.substr(chunkSize, chunkSize + 2).toStr() != "\r\n")
        throw HttpError(BadRequest, "Bad Request");
    else
    {
        writeToFile(this->readingBuffer.substr(0, chunkSize).toStr(), this->tmpFiles[0]);
        counter += chunkSize;
        if (counter > RequestParser::serverConfig.max_body_size)
            throw HttpError(BadRequest, "Bad Request");
    }
}

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
        else if (!isCGI)
            throw HttpError(Forbidden, "Forbidden");
        else
            // handle cgi
            // isCGI = isCGI;
            std::cout << "handle cgi for post!" << std::endl;
    }
    else
    {
        writeToFile(this->readingBuffer.toStr(), this->tmpFiles[0]);
        counter += readingBuffer.size();
        if (counter > RequestParser::serverConfig.max_body_size || counter > contentLength)
            throw HttpError(BadRequest, "Bad Request");
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