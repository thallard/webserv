#ifndef Core_HPP
# define Core_HPP

#include "Utils.hpp"
#include "Server.hpp"
#include "Worker.hpp"
using namespace std;



class Worker;
class Core
{
	private:
		int		_count_workers;
		// Request request;
		vector<Server *> _servers;
		vector<_t_preServ> _pre_Serv;
		map<int, Worker *> _workers;
		Core();
		void setWorkers(string);
		
		void parseServer(int, string, string, int *);
		pair<string, vector<string> > parseMethod(map<string, string>, string, string, int);
		vector<string> parseMethod(string, string, int);

		int _i;
	public:
		//Server *test;
		Core(string);
		Core &operator=(const Core &);
		~Core();

		void setIdServer(int i) { _i = i; };
		int getIdServer() { return _i;};
		// Workers
		int getCountWorkers();
		map<int, Worker *> &getWorkers();

		vector<Server *> getServers();
		Server *getAt(int n) { return _servers[n];};
	

		void run(map<int, Worker *> & workers, int count);

};

#endif