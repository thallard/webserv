#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "Utils.hpp"
#include "Server.hpp"
#include "Worker.hpp"
using namespace std;

class Worker;
class Config
{
	private:
		int		_count_workers;
		// Request request;
		vector<Server> _servers;
		map<int, Worker *> _workers;
		Config();
		void setWorkers(string);
		Server parseServer(int, string, char *, int *);
	public:
		Config(char *);
		
		// Workers
		int getCountWorkers();
		map<int, Worker *> getWorkers();

		vector<Server> getServers();
		~Config();
};

#endif