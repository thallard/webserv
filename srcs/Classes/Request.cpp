#include "Request.hpp"

Request::Request(/* args */)
{
	socket = 0;
	server = NULL;
	header = NULL;
}

Request::Request(int sock, Server serv, Headers head)
{
	socket = sock;
	server = serv;
	header = head;
}

Request::~Request()
{
}

int Request::getSocket()
{
	return socket;
}

Server Request::getServer()
{
	return server;
}

Headers Request::getHeader()
{
	return header;
}