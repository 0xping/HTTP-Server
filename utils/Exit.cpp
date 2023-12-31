#include "utils.hpp"

void cerrAndExit(std::string err, int s){
    std::cerr << err << std::endl;
    std::exit(s);
}
