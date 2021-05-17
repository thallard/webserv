#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "Utils.hpp"
#include "Server.hpp"
#include "Worker.hpp"
using namespace std;

typedef struct _s_preServ
{
	int id;
	int port;
	string name;
	string root;
	map<int, string> err;
	map<string, map<string, string> > loc;
} 			_t_preServ;

class Worker;
class Config
{
	private:
		int		_count_workers;
		// Request request;
		vector<Server *> _servers;
		vector<_t_preServ> _pre_Serv;
		map<int, Worker *> _workers;
		Config();
		void setWorkers(string);
		void parseServer(int, string, string, int *);
	public:
		//Server *test;
		Config(string);
		
		// Workers
		int getCountWorkers();
		map<int, Worker *> getWorkers();

		vector<Server *> getServers();
		Server *getAt(int n) { return _servers[n];};
		~Config();
};

#endif