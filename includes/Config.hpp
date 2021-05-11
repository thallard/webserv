#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "Utils.hpp"
#include "Server.hpp"
using namespace std;
class Config
{
	private:
		map<Server, map<string, map<string, string> > > _confs;
		Config();
	public:
		Config(char *);
		~Config();
};

#endif