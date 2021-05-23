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
		vector<Server *> _servers;
		vector<_t_preServ> _pre_Serv;
		map<int, Worker *> _workers;
		Config();
		void setWorkers(string);
		void parseServer(int, string, string, int *);
		pair<string, vector<string> > parseMethod(map<string, string>, string, string, int);
		vector<string> parseMethod(string, string, int);
	public:
		//Server *test;
		Config(string);
		
		// Workers
		int getCountWorkers();
		map<int, Worker *> &getWorkers();

		vector<Server *> getServers();
		Server *getAt(int n) { return _servers[n];};
		~Config();
};

#endif