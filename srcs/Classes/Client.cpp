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

Client &Client::operator=(const Client & ref)
{
	_id = ref._id;
	_content = ref._content;
	_socket = ref._socket;
	return *this;
}

Client::~Client()
{
}

// Read the content on the current socket
bool Client::readContent()
{
	char buf[65535];
	int nbytes_read = 0;
	// bzero(buf, 65535);
	nbytes_read = recv(getSocket(), buf, 500, 0);
		nbytes_read = recv(getSocket(), buf + nbytes_read, 500, 0);
	// while () > 1)
	// {
	// 		_content += buf;
	// 		// cout << _content << endl;
	// 		// cout << "nbytes read = " << nbytes_read << endl;
	// }
	cout << buf << endl;
	cout << "je sors du readcontent\n";
	if (nbytes_read < 0)
		return false;
	return true;
}