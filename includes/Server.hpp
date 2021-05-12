#ifndef SERVER_HPP
#define SERVER_HPP
#include "Utils.hpp"

using namespace std;
class Server
{		
	public:
		typedef struct sockaddr_in sock_addr;

	private:
	Server();
		sock_addr _serv_addr;
		sock_addr _cli_addr;

		int _id;
		int _socket;

		string _name;
		string _root;

		map<string, map<string, string> > _locations;
		map<int, string> _error_pages;
	public:
		Server(int);
		Server &operator=(Server const & ref);
		~Server();
		int getSocket() { return _socket; };
		int getId() { return _id; };

		string getRoot() { return _root; };
		string getName() { return _name; };

		sockaddr_in getServAddr() { return _serv_addr; };
		sockaddr_in getCliAddr() { return _cli_addr; };
		sockaddr_in *getCliAddr_ptr() { return &_cli_addr;};

		void setId(int n) {_id = n;};
		void setName(string name) {_name = name;};
		void setRoot(string root) {_root = root;};
		void setErrorPages(map<int, string> pages) {_error_pages = pages;};
		void setLocations(map<string, map<string, string> > loc) {_locations = loc;};
};

#endif