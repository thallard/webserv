#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "Utils.hpp"

class Client
{
private:
	int			_socket;
	int			_id;
	string		_content;

public:
	Client();
	Client(int, int);
	Client(const Client &);
	Client &operator=(const Client &);
	~Client();

	// Setters and getters
	int getSocket() { return _socket; }
	int getId() { return _id; }
	string getContent() { return _content; }
	
	void setSocket(int socket) { _socket = socket; }
	void setId(int id) { _id = id; }
	void setContent(string content) { _content = content; }

	bool exists(int socket);
	bool readContent();
};




#endif