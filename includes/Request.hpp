#ifndef REQUEST_HPP
#define REQUEST_HPP
#include "Utils.hpp"
#include "Server.hpp"
#include "Headers.hpp"

class Request
{
private:
	int socket;
	Server server;
	Headers header;
	Request();

public:
	Request(int sock, Server serv, Headers header);
	~Request();
	int getSocket();
	Server getServer();
	Headers getHeader();
};




#endif