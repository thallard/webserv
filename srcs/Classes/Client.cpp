#include "Client.hpp"


Client::Client()
{
}

Client::Client(int id, int socket)
{
    _id = id;
    _socket = socket;
}

Client::Client(const Client & ref)
{
    (void)ref;
}

Client::~Client()
{
}

