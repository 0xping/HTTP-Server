#include "ClientHandler.hpp"

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
	else{
		if (!std::freopen(cgioutput.c_str(), "w+", stdout))
			std::exit(1);
		if (message.method == "POST")
		{
			if (!std::freopen(tmpFiles[0].c_str(), "r", stdin))
				std::exit(1);
		}
		const char *args[] = {CGIpath.c_str(), fullLocation.c_str(), postedFileName.c_str(), NULL};
		const char *env[] = {query.c_str(), NULL};

		execve(CGIpath.c_str(), (char *const *)args, (char *const *)env);
		std::exit(1);
	}
}

void ClientHandler::checkCGI(){
	int status;
	if (waitpid(CGIpid, &status, WNOHANG) == 0){
		std::time_t endTime = std::time(0);
		int elapsedTime = endTime - CGIstartTime;
		if (elapsedTime > 5){
			kill(CGIpid, SIGTERM);
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