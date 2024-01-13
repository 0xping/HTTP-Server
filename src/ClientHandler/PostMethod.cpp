#include "ClientHandler.hpp"

void writeToFile(const std::string &data, const std::string& fileName)
{
    if (!freopen(fileName.c_str(), "a", stdout))
        throw HttpError(InternalServerError, "Internal Server Error 1");
    std::cout << data;
    if (!freopen("/dev/tty", "a", stdout))
        throw HttpError(InternalServerError, "Internal Server Error 2");
    // std::cout << fileName << std::endl;
}  

void ClientHandler::mutiple_part_handler()
{

}

void ClientHandler::chunked_handler()
{
    // std::cout << "hi" << std::endl;
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
            // std::cout << readingBuffer.substr(0, readingBuffer.find("\r\n")).toStr() << "|" << std::endl;
            if (!isValidBase(readingBuffer.substr(0, readingBuffer.find("\r\n")).toStr(), chunkSize, 16))
                throw HttpError(BadRequest, "Bad Request 3");
            // std::cout << chunkSize << " " << readingBuffer.substr(0, readingBuffer.find("\r\n")).toStr() << std::endl;
            if (!chunkSize)
            {
                if (readingBuffer.find("\r\n\r\n") != std::string::npos)
                {
                    // std::cout << "start chunked" << std::endl;
                    writeToFile(this->readingBuffer.substr(0, chunkSize).toStr(), this->tmpFiles[0]);
                    this->tmpFiles.push_back(generateUniqueFileName());
                    writeToFile(postHtmlResponseGenerator(tmpFiles), this->tmpFiles[this->tmpFiles.size() - 1]);
                    setResponseParams("200", "OK", "", this->tmpFiles[this->tmpFiles.size() - 1]);
                    tmpFiles.erase(tmpFiles.begin(), tmpFiles.end() - 1);
                    // std::cout << "end chunked" << std::endl;
                    return;
                }
                else
                    return;
            }
            else
            {
                counter += readingBuffer.find("\r\n") + 2;
                readingBuffer = readingBuffer.substr(readingBuffer.find("\r\n") + 2, readingBuffer.size());
            }
        }
        else
            return;
    }
    // std::cout << "==|" <<replaceNewlineWithLiteral(this->readingBuffer.toStr()) << "|==" << std::endl;
    if (this->readingBuffer.size() <= chunkSize + 1)
        return;
    // std::cout << "here: " << chunkSize << std::endl;
    // std::cout << "|" <<replaceNewlineWithLiteral(this->readingBuffer.substr(chunkSize, 2).toStr()) << "|" << std::endl;
    if (this->readingBuffer.substr(chunkSize, 2).toStr() != "\r\n")
        throw HttpError(BadRequest, "Bad Request 4");
    else
    {
        writeToFile(this->readingBuffer.substr(0, chunkSize).toStr(), this->tmpFiles[0]);
        this->readingBuffer = this->readingBuffer.substr(chunkSize + 2, this->readingBuffer.size());
        counter += chunkSize;
        chunkSize = 0;
        if (counter > RequestParser::serverConfig.max_body_size)
            throw HttpError(BadRequest, "Bad Request 5");
        chunked_handler();
    }
}

void ClientHandler::regular_data_handler()
{
    // std::cout << "wtf 1" << std::endl;
    if (!this->tmpFiles.size())
    {
    // std::cout << "wtf 2" << std::endl;

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
    // std::cout << "wtf 3" << std::endl;

        // std::cout << replaceNewlineWithLiteral(this->readingBuffer.toStr()) << std::endl;
        writeToFile(this->readingBuffer.toStr(), this->tmpFiles[0]);
        counter += readingBuffer.size();
        // std::cout << counter << " " << contentLength << std::endl;
        this->readingBuffer.erase(0, this->readingBuffer.size());
        if (counter > RequestParser::serverConfig.max_body_size || counter > contentLength)
            throw HttpError(BadRequest, "Bad Request 6");
        if (counter == contentLength)
        {
            // std::cout << "hello" << std::endl;
            this->tmpFiles.push_back(generateUniqueFileName());
            writeToFile(postHtmlResponseGenerator(tmpFiles), this->tmpFiles[this->tmpFiles.size() - 1]);
            setResponseParams("200", "OK", "", this->tmpFiles[this->tmpFiles.size() - 1]);
            // std::cout << this->tmpFiles[this->tmpFiles.size() - 1] << std::endl;
            // sleep(50);
            tmpFiles.erase(tmpFiles.begin(), tmpFiles.end() - 1);
        }
}

void ClientHandler::PostMethod()
{
    if (in)
        readFromSocket();
    else
        in = 1;

    if (message.headers.find("Transfer-Encoding") != message.headers.end())
    {
        // implement chunked
        chunked_handler();
    }
    else
    {
        // throw HttpError(BadRequest, "Bad Request");
        regular_data_handler();
    }
    // else
    // {
    //     if (message.headers.find("Content-Type") != message.headers.end())
    //     {
    //         // store it by mime type or .tmp
    //     }
    //     else
    //     {
    //         // read and ignore
    //     }
    // }
}