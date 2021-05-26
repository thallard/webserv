#ifndef SERVER_HPP
#define SERVER_HPP
#include "Utils.hpp"
#include "Worker.hpp"
#include "Headers.hpp"
#include "Client.hpp"

using namespace std;

typedef struct s_file
{
	string content;
	size_t size;
}				t_file;



class Worker;
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
		t_loc *root;

		map<string, map<string, string> > _locations;
		map<string, vector<string> > _locations_methods;
		map<int, string> _error_pages;
		map<int, Worker *> _workers;
		list<Client> _clients;
		vector<string> _allowed;

		fd_set _clients_fd;

		int _count_requests;
		pthread_t *thread;
		pthread_mutex_t *_logger;
	public:
		Server(int);
		Server(_t_preServ, pthread_mutex_t *);
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

		fd_set getClientsFD() { return _clients_fd; }
		fd_set *getClientsFD_ptr() { return &_clients_fd; }

		pthread_t *getThread() { return thread; }
		pthread_mutex_t *getLogger() {return _logger;};
		void run(map<int, Worker *> &);
		void error(const char *);
		void log(string);
		bool check_methods(map<string, string>);
		void handle_request(Client);

		string GET(map<string, string>, int);
		string POST(map<string, string>, int);
		string HEAD(map<string, string>, int);
		string PUT(map<string, string>, int);
		string SEND_ERROR(int , const char *);
		
		t_file getFile(string);
		pair< string, map<string, string> > getConfLoc(string);

		// Clients part
		list<Client> &getClients() { return _clients; }
		int exists(int socket, list<Client> &clients);

		// Workers
		int findAvailableWorker(map<int, Worker *> &);
	
		


	

		
};

#endif