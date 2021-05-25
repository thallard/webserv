#ifndef CORE_HPP
# define CORE_HPP

#include "Utils.hpp"
#include "Server.hpp"
#include "Worker.hpp"
#include "Client.hpp"
using namespace std;



class Worker;
class Core
{
	private:
		int		_count_workers;
		vector<Server *> _servers;
		vector<_t_preServ> _pre_Serv;
		map<int, Worker *> _workers;
		list<Client> _clients;
		Core();
		void setWorkers(string);
		
		void parseServer(int, string, string, int *);
		pair<string, vector<string> > parseMethod(map<string, string>, string, string, int);
		vector<string> parseMethod(string, string, int);

		int _i;
	public:
		Core(string);
		Core &operator=(const Core &);
		~Core();

		
		// Workers
		int getCountWorkers();
		map<int, Worker *> &getWorkers();

	
		// Server part
		void run(map<int, Worker *> & workers, int count);
		vector<Server *> getServers();
		Server *getAt(int n) { return _servers[n]; }
		void setIdServer(int i) { _i = i; }
		int getIdServer() { return _i; }

		// Client part
		bool exists(int socket);

};

#endif