#ifndef SERVER_HPP
#define SERVER_HPP
#include "Utils.hpp"

class Server
{
	private:
		Server();
	public:
		typedef struct sockaddr_in sock_addr;

	private:
		sock_addr _serv_addr;
		sock_addr _cli_addr;
		int _socket;
	public:
		Server(int);
		~Server();
	public:
		int getSocket() { return _socket; };
		sockaddr_in getServAddr() { return _serv_addr; };
		sockaddr_in getCliAddr() { return _cli_addr; };
		sockaddr_in *getCliAddr_ptr() { return &_cli_addr;};
};

#endif