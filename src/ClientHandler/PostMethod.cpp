#include "ClientHandler.hpp"

void writeToFile(const std::string &data, const std::string& fileName)
{
    // std::cout << fileName << std::endl;
    if (!freopen(fileName.c_str(), "ab", stdout))
        throw HttpError(InternalServerError, "Internal Server Error");
    std::cout << data;
    if (!freopen("/dev/tty", "ab", stdout))
        throw HttpError(InternalServerError, "Internal Server Error");
}  

bool checkMultipart(std::string& str, std::string& boundary)
{
    size_t sep_pos = str.find(";");
    if (sep_pos == std::string::npos)
        return false;
    if (strtrim(str.substr(0, sep_pos)) != "multipart/form-data")
        return false;
    std::string bound = strtrim(str.substr(sep_pos + 1, str.size()));
    sep_pos = bound.find("=");
    if (sep_pos == std::string::npos)
        return false;
    if (strtrim(strtrim(bound.substr(0, sep_pos))) != "boundary")
        return false;
    boundary = strtrim(bound.substr(sep_pos + 1, bound.size()));
    if (boundary == "")
        return false;
    boundary = "--" + boundary;
    return true;
}

bool isContentDispositionValid(const std::string &str, std::string &fileName)
{
    size_t pos_1 = str.find("filename=");
    size_t pos_2;
    if (pos_1 != std::string::npos && (pos_2 = str.substr(pos_1 + 10, str.size()).find("\"")))
    {
        fileName = str.substr(pos_1 + 10, pos_2);
        if (fileName == "")
            return false;
        return true;
    }
    return false;
}

void ClientHandler::MutiplePartHandler()
{
    size_t pos;

    // if (!isCGIfile && !isDir)
    //     throw HttpError(Forbidden, "Forbidden");
    if (state == startBound)
    {
        // std::cout << "->startBound: |" << replaceNewlineWithLiteral(this->readingBuffer.toStr()) << "|" <<  std::endl;
        // std::cout << boundary << std::endl;
        // std::cout << replaceNewlineWithLiteral(this->readingBuffer.toStr()) << " size: " << this->readingBuffer.size()<< std::endl;
        if (this->readingBuffer.size() < boundary.size() + 4)
            return;
        // std::string suff = "";
        // if (firstboundary)
        // {
        //     suff = "--";
        //     firstboundary = false;
        // }
        if (this->readingBuffer.substr(0, boundary.size() + 2).toStr() !=  boundary + "\r\n")
            throw HttpError(BadRequest, "Bad Request");
        else
        {
            this->counter += boundary.size() + 2;
            this->readingBuffer = this->readingBuffer.substr(boundary.size() + 2, this->readingBuffer.size());
            this->state = ContentDisposition;
        }
    }
    if (state == ContentDisposition)
    {
        // this->location.path
        // std::cout << "->ContentDisposition: |" << replaceNewlineWithLiteral(this->readingBuffer.toStr()) << "|" << std::endl;
        std::string gigaStr;
        pos = this->readingBuffer.find("\r\n\r\n");
        // std::cout << "hello: " << this->readingBuffer.substr(pos, readingBuffer.size()).toStr() << std::endl;
        if (pos == std::string::npos)
            return;
        if (isContentDispositionValid(this->readingBuffer.substr(0, pos).toStr(), gigaStr))
        {
            if (fileExists((RequestParser::upload_path + "/" + gigaStr).c_str()))
                throw HttpError(Conflict, "Conflict");
            else
                this->tmpFiles.push_back(RequestParser::upload_path + "/" + gigaStr);
            this->counter += pos + 4;
            this->readingBuffer = this->readingBuffer.substr(pos + 4, this->readingBuffer.size());
            state = FileContent;
        }
        else
            throw HttpError(BadRequest, "Bad Request");
    }
    if (state == FileContent)
    {
        // std::cout << "Boundary: |" << boundary << "|" << std::endl;
        // std::cout << "->FileContent: |" << replaceNewlineWithLiteral(this->readingBuffer.toStr()) << "|" << std::endl;
        pos = this->readingBuffer.find("\r\n" + boundary);
        if (pos != std::string::npos)
        {
            // std::cout << "here 1\n";
            writeToFile(this->readingBuffer.substr(0, pos).toStr(), this->tmpFiles[this->tmpFiles.size() - 1]);
            this->counter += pos + 2;
            this->readingBuffer = this->readingBuffer.substr(pos + 2, this->readingBuffer.size());
            state = EndBound;
        }
        else
        {//----------------------------005948816781938767311044
         //--------------------------005948816781938767311044
            // std::cout << "here 2\n";
            size_t length = this->readingBuffer.size();
            writeToFile(this->readingBuffer.substr(0, length - ("\r\n" + boundary).size()).toStr(), this->tmpFiles[this->tmpFiles.size() - 1]);
            this->counter += length - ("\r\n" + boundary).size();
            this->readingBuffer = this->readingBuffer.substr(length - ("\r\n" + boundary).size(), this->readingBuffer.size());
        }
    }
    if (state == EndBound)
    {
        // std::cout << "->EndBound: |" << replaceNewlineWithLiteral(this->readingBuffer.toStr()) << "|" << std::endl;
        if (this->readingBuffer.size() < boundary.size() + 4)
            return;
        if (this->readingBuffer.substr(0, boundary.size() + 4).toStr() == boundary + "--\r\n")
        {
            if (this->counter + boundary.size() + 4 > RequestParser::serverConfig.max_body_size)
                throw HttpError(PayloadTooLarge, "Payload Too Large");
            if (this->counter + boundary.size() + 4 > this->contentLength)
                throw HttpError(BadRequest, "Bad Request");
            this->tmpFiles.push_back(generateUniqueFileName());
            writeToFile(postHtmlResponseGenerator(tmpFiles), this->tmpFiles[this->tmpFiles.size() - 1]);
            setResponseParams("201", "OK", "", this->tmpFiles[this->tmpFiles.size() - 1]);
            // firstboundary = 1;
            tmpFiles.erase(tmpFiles.begin(), tmpFiles.end() - 1);
        }
        else
        {
            state = startBound;
            MutiplePartHandler();
        }
    }
        // std::cout << "counter: " << counter << ", max body size: " << RequestParser::serverConfig.max_body_size << ", contentLength: " << this->contentLength << std::endl;
    if (this->counter > RequestParser::serverConfig.max_body_size)
        throw HttpError(PayloadTooLarge, "Payload Too Large");
    if (this->counter > this->contentLength)
        throw HttpError(BadRequest, "Bad Request");
    
}

void ClientHandler::ChunkedHandler()
{
    // std::cout << "hi" << std::endl;
    if (!this->tmpFiles.size())
    {
        if (message.headers.find("Content-Type") != message.headers.end())
            this->tmpFiles.push_back(generateUniqueFileName(RequestParser::upload_path, getExtensionPost(message.headers["Content-Type"])));
        else
            this->tmpFiles.push_back(generateUniqueFileName(RequestParser::upload_path, ".tmp"));
        // else if (!isCGIfile)
        //     throw HttpError(Forbidden, "Forbidden");
        // else
        //     // handle cgi
        //     // isCGIfile = isCGIfile;
        //     std::cout << "handle cgi for post!" << std::endl;
    }
    if (!chunkSize)
    {
        if (readingBuffer.find("\r\n") != std::string::npos)
        {
            // std::cout << readingBuffer.substr(0, readingBuffer.find("\r\n")).toStr() << "|" << std::endl;
            if (!isValidBase(readingBuffer.substr(0, readingBuffer.find("\r\n")).toStr(), chunkSize, 16))
                throw HttpError(BadRequest, "Bad Request");
            // std::cout << chunkSize << " " << readingBuffer.substr(0, readingBuffer.find("\r\n")).toStr() << std::endl;
            if (!chunkSize)
            {
                if (readingBuffer.find("\r\n\r\n") != std::string::npos)
                {
                    // std::cout << "start chunked" << std::endl;
                    writeToFile(this->readingBuffer.substr(0, chunkSize).toStr(), this->tmpFiles[0]);
                    if (isCGIfile)
                    {
                        execCGI();
                        return;
                    }
                    this->tmpFiles.push_back(generateUniqueFileName());
                    writeToFile(postHtmlResponseGenerator(tmpFiles), this->tmpFiles[this->tmpFiles.size() - 1]);
                    setResponseParams("201", "OK", "", this->tmpFiles[this->tmpFiles.size() - 1]);
                    tmpFiles.erase(tmpFiles.begin(), tmpFiles.end() - 1);
                    // std::cout << "end chunked" << std::endl;
                }
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
        throw HttpError(BadRequest, "Bad Request");
    else
    {
        writeToFile(this->readingBuffer.substr(0, chunkSize).toStr(), this->tmpFiles[0]);
        this->readingBuffer = this->readingBuffer.substr(chunkSize + 2, this->readingBuffer.size());
        counter += chunkSize;
        chunkSize = 0;
        if (counter > RequestParser::serverConfig.max_body_size)
            throw HttpError(BadRequest, "Bad Request");
        ChunkedHandler();
    }
}

void ClientHandler::RegularDataHandler()
{
    // std::cout << "wtf 1" << std::endl;
    if (!this->tmpFiles.size())
    {
    // std::cout << "wtf 2" << std::endl;

        if (message.headers.find("Content-Type") != message.headers.end())
            this->tmpFiles.push_back(generateUniqueFileName(RequestParser::upload_path, getExtensionPost(message.headers["Content-Type"])));
        else
            this->tmpFiles.push_back(generateUniqueFileName(RequestParser::upload_path, ".tmp"));
        // }
        // else if (!isCGI)
        //     throw HttpError(Forbidden, "Forbidden");
        // else
        //     // handle cgi
        //     // isCGI = isCGI;
        //     std::cout << "handle cgi for post!" << std::endl;
    }
    // std::cout << "wtf 3" << std::endl;

        // std::cout << replaceNewlineWithLiteral(this->readingBuffer.toStr()) << std::endl;
        writeToFile(this->readingBuffer.toStr(), this->tmpFiles[0]);
        counter += readingBuffer.size();
        // std::cout << counter << " " << contentLength << std::endl;
        this->readingBuffer.erase(0, this->readingBuffer.size());
        if (this->counter > RequestParser::serverConfig.max_body_size)
        throw HttpError(PayloadTooLarge, "Payload Too Large");
        if (this->counter > this->contentLength)
            throw HttpError(BadRequest, "Bad Request");
        if (counter == contentLength)
        {
            // std::cout << "hello" << std::endl;
            if (isCGIfile)
            {
                execCGI();
                return;
            }
            this->tmpFiles.push_back(generateUniqueFileName());
            writeToFile(postHtmlResponseGenerator(tmpFiles), this->tmpFiles[this->tmpFiles.size() - 1]);
            setResponseParams("201", "OK", "", this->tmpFiles[this->tmpFiles.size() - 1]);
            // std::cout << this->tmpFiles[this->tmpFiles.size() - 1] << std::endl;
            // sleep(50);
            tmpFiles.erase(tmpFiles.begin(), tmpFiles.end() - 1);
        }
}



void ClientHandler::PostMethod()
{
    // std::cout << "================================" << std::endl;
    if (in)
        readFromSocket();
    else
        in = 1;

    if (message.headers.find("Transfer-Encoding") != message.headers.end())
    {
        // implement chunked
        ChunkedHandler();
    }
    else if (message.headers.find("Content-Type") != message.headers.end() && checkMultipart(message.headers["Content-Type"], boundary) && !isCGIfile)
    {
        // std::cout << boundary << std::endl;
        // std::cout << replaceNewlineWithLiteral(this->readingBuffer.toStr()) << std::endl;
        // readingBuffer.erase(0, readingBuffer.size());
        MutiplePartHandler();
    }
    else
    {
        // throw HttpError(BadRequest, "Bad Request");
        RegularDataHandler();
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
