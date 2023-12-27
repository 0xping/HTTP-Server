#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

class Config
{
	private:
		/* data */
	public:
		std::string port;
		std::string host;
		Config(std::string host,std::string port)
		{
			this->host=host;
			this->port=port;
		}

};

#endif //CONFIG_HPP