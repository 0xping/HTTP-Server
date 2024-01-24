#include "ClientHandler.hpp"
char* createCString(const std::string& str) {
	char* result = new char[str.length() + 1];
	std::strcpy(result, str.c_str());
	return result;
}

void deleteCStrings(std::vector<char*>& cStrings) {
	for (size_t i = 0; i < cStrings.size(); ++i) {
		delete[] cStrings[i];
	}
}

void ClientHandler::execCGI(){
	std::string cgioutput = generateUniqueFileName();
	tmpFiles.push_back(cgioutput);

	pid_t pid = fork();
	if (pid){
		if (pid < 0)
			throw HttpError(InternalServerError, "Internal Server Error");

		this->CGIpid = pid;
		this->CGIoutput = cgioutput;
		this->monitorCGI = 1;
		this->CGIstartTime = std::time(0);
		status = Sending;
	}
	else
	{
		if (!std::freopen(cgioutput.c_str(), "w+b", stdout))
			std::exit(1);
		if (message.method == "POST")
		{
			if (!std::freopen(tmpFiles[0].c_str(), "rb", stdin))
				std::exit(1);
		}
		std::string cookie = "";
		if (message.headers.find("Cookie") != message.headers.end())
			cookie = message.headers["Cookie"];
		const char *args[] = {CGIpath.c_str(), fullLocation.c_str(), postedFileName.c_str(), NULL};
		std::vector<char*> env;

		env.push_back(createCString("QUERY_STRING=" + query));
		env.push_back(createCString("HTTP_COOKIE=" + cookie));
		env.push_back(createCString("REQUEST_METHOD=" + message.method));
		env.push_back(NULL);

		execve(CGIpath.c_str(), const_cast<char* const*>(args), const_cast<char* const*>(&env[0]));
		deleteCStrings(env);
		std::exit(1);
	}
}

void ClientHandler::checkCGI(){
	int status;
	if (waitpid(CGIpid, &status, WNOHANG) == 0){
		std::time_t endTime = std::time(0);
		int elapsedTime = endTime - CGIstartTime;
		if (elapsedTime > 5){
			kill(CGIpid, SIGKILL);
			waitpid(CGIpid, &status, 0);
			monitorCGI = 0;
			throw HttpError(RequestTimeOut, "Request Time Out");
		}
	}
	else
	{
		monitorCGI = 0;
		if (WIFEXITED(status)){
			if (!WEXITSTATUS(status))
				setResponseParams("200", "OK", "", CGIoutput, true);
			else
				throw HttpError(InternalServerError, "Internal Server Error");
		}
		else if (WIFSIGNALED(status))
			throw HttpError(InternalServerError, "Internal Server Error");
	}
}