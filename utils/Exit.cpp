#include "utils.hpp"

void cerrAndExit(std::string err, int s){
    std::cerr << err << std::endl; // print err to standard error and exit with s
    std::exit(s);
}
