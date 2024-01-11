#include "ClientHandler.hpp"

void ClientHandler::execCGI(){
	std::string cgioutput = generateUniqueFileName();
    tmpFiles.push_back(cgioutput);
	
	pid_t pid = fork(); 
	if (pid){
		if (pid < 0)
			throw HttpError(InternalServerError, "Internal Server Error");
		int status;
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
		int fd = open(cgioutput.c_str(), O_RDWR | O_CREAT, 0777);
		if (fd < 0)
			exit(1);		

		const char *args[] = {CGIpath.c_str(), fullLocation.c_str(), postedFileName.c_str(), NULL};
		const char *env[] = {query.c_str(), NULL};
		if (dup2(fd,1) == -1)
			exit(1);

		execve(CGIpath.c_str(), (char *const *)args, (char *const *)env);
		exit(1);
	}
}