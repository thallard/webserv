#ifndef SERVER_HPP
#define SERVER_HPP
#include "Utils.hpp"
#include "Worker.hpp"

using namespace std;

typedef struct s_file
{
	string content;
	size_t size;
}				t_file;

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
		string _index;

		map<string, map<string, string> > _locations;
		map<int, string> _error_pages;

		fd_set _write_fds;
		fd_set _read_fds;

		int _count_requests;
	public:
		Server(int);
		Server(int, string, string, map<int, string>, map<string, map<string, string> >);
		Server(int, int, string, string, map<int, string>, map<string, map<string, string> >, string);
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

		fd_set getWriteFD() { return _write_fds;};
		fd_set getReadFD() { return _read_fds;};
		fd_set *getWriteFD_ptr() { return &_write_fds;};
		fd_set *getReadFD_ptr() { return &_read_fds;};

		void run(map<int, Worker *>, int);
	private:
		void handle_request(int);
		void error(const char *);
		void log(string);

		string GET(map<string, string>);
		string POST(map<string, string>);
		string HEAD(map<string, string>);

		t_file getFile(string);
};

#endif