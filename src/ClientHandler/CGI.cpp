#include "ClientHandler.hpp"

void ClientHandler::execCGI(){
	std::string cgioutput = generateUniqueFileName();
    tmpFiles.push_back(cgioutput);
	
	pid_t pid = fork(); 
	if (pid){
		int status;
		
		if (pid < 0)
			throw HttpError(InternalServerError, "Internal Server Error");
		
		clock_t startTime = clock();

		while (waitpid(pid, &status, WNOHANG) == 0){
			clock_t endTime = clock();

			double elapsedTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
			if (elapsedTime >= 30){				
				kill(pid, SIGTERM);
				throw HttpError(InternalServerError, "Internal Server Error");
			}
		}

    	if (WIFEXITED(status)){
			if (!WEXITSTATUS(status))
				setResponseParams("200", "OK", "", cgioutput, true);
			else
				throw HttpError(InternalServerError, "Internal Server Error");
		}
		else if (WIFSIGNALED(status))
			throw HttpError(InternalServerError, "Internal Server Error");
    	
	}
	else{
		if (!std::freopen(cgioutput.c_str(), "w+", stdout))
			std::exit(1);	

		const char *args[] = {CGIpath.c_str(), fullLocation.c_str(), postedFileName.c_str(), NULL};
		const char *env[] = {query.c_str(), NULL};

		execve(CGIpath.c_str(), (char *const *)args, (char *const *)env);
		std::exit(1);
	}
}