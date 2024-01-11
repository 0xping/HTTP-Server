#include "utils.hpp"

bool fileExists(const std::string& name)
{
    std::ifstream file(name.c_str());
    return file.good();
}

std::string generateUniqueFileName(const std::string& directory = "/tmp")
{
    char tmpname[100];
    std::ostringstream ss;
    unsigned int iter = 0;
    do
    {
        ss.str("");
        ss.clear();
        std::time_t currentTime = std::time(NULL);
        struct std::tm *localTime = std::localtime(&currentTime);
        std::srand(static_cast<unsigned int>(currentTime) + iter);
        ss << std::rand() % 100;
        std::string rand_int;
        std::strftime(tmpname, sizeof(tmpname), "%Y%m%d%H%M%S", localTime);
        iter++;
    } while (fileExists(directory + '/' + std::string(tmpname) + ss.str()));
    
    return directory + '/' + std::string(tmpname) + ss.str();
}