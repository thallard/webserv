#include "Server.hpp"

Server::Server(int port)
{
	//Socket()
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	cout << "created socket " << _socket << ":" << port << endl;
	if (_socket < 0)
		perror("ERROR opening socket");

	//IOCTL()
	const int opt = 1;
	setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

	//BIND()
	bzero((char *)&_serv_addr, sizeof(_serv_addr));
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = INADDR_ANY;
	_serv_addr.sin_port = htons(port);
	if (bind(_socket, (struct sockaddr *)&_serv_addr,
			sizeof(_serv_addr)) < 0)
		perror("ERROR on binding");

	listen(_socket, 4096);

	FD_ZERO(&_write_fds);
	FD_ZERO(&_read_fds);

	FD_SET(_socket, &_write_fds);
	FD_SET(_socket, &_read_fds);
}

Server::Server(int id, int port, string name, string root, map<int, string> err_pages, map<string, map<string, string> > location)
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	cout << "created socket " << _socket << ":" << port << endl;
	if (_socket < 0)
		perror("ERROR opening socket");

	//IOCTL()
	const int opt = 1;
	setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

	//BIND()
	bzero((char *)&_serv_addr, sizeof(_serv_addr));
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = INADDR_ANY;
	_serv_addr.sin_port = htons(port);
	if (bind(_socket, (struct sockaddr *)&_serv_addr,
			sizeof(_serv_addr)) < 0)
		perror("ERROR on binding");

	listen(_socket, 4096);

	FD_ZERO(&_write_fds);
	FD_ZERO(&_read_fds);

	FD_SET(_socket, &_write_fds);
	FD_SET(_socket, &_read_fds);

	_id = id;
	_name = name;
	_root = root;
	_error_pages = err_pages;
	_locations = location;
}

Server::Server(int id, string name, string root, map<int, string> err_pages, map<string, map<string, string> > location)
{
	_id = id;
	_name = name;
	_root = root;
	_error_pages = err_pages;
	_locations = location;
}

Server::~Server()
{
	close(_socket);
}

Server &Server::operator=(Server const & ref)
{

	_serv_addr = ref._serv_addr;
	_cli_addr = ref._cli_addr;

	_id = ref._id;
	_socket = ref._socket;

	_name = ref._name;
	_root = ref._root;

	_locations = ref._locations;
	_error_pages = ref._error_pages;


	return *this;
}

void Server::setPort(int port)
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	cout << "created socket " << _socket << ":" << port << endl;
	if (_socket < 0)
		perror("ERROR opening socket");

	//IOCTL()
	const int opt = 1;
	setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

	//BIND()
	bzero((char *)&_serv_addr, sizeof(_serv_addr));
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = INADDR_ANY;
	_serv_addr.sin_port = htons(port);
	if (bind(_socket, (struct sockaddr *)&_serv_addr,
			sizeof(_serv_addr)) < 0)
		perror("ERROR on binding");

	listen(_socket, 4096);

	FD_ZERO(&_write_fds);
	FD_ZERO(&_read_fds);

	FD_SET(_socket, &_write_fds);
	FD_SET(_socket, &_read_fds);
}

void Server::run(map<int, Worker *> workers, int count)
{
	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = 10;
	_count_requests = 0;
	(void)workers[count];

	socklen_t clilen = sizeof(_cli_addr);

	while(1)
	{
		fd_set write_fds2 =  _write_fds;
		fd_set read_fds2 =  _read_fds;

		if (select(_socket + 1, &read_fds2, &write_fds2, NULL, &tv) > 0)
		{
			//int i = 0;
			/*while (1)
			{
				i = 0;
				while (workers.find(i)->second->getStatus() && i < count - 1)
					i++;
				if (workers.find(i)->second->getStatus())
					break;
			}*/
			if (!FD_ISSET(_socket, &_write_fds) || !FD_ISSET(_socket, &_read_fds))
				error("ERROR non-set socket");
			int newsockfd = accept(_socket, (struct sockaddr *)&_cli_addr, &clilen);
			if (newsockfd < 0)
				error("ERROR on accept");
			do_s(newsockfd);
			close(newsockfd);
		}
		else
			dprintf(1, "No response pendant 10sec\n");
	}
}

#include "Headers.hpp"

void Server::do_s(int sock)
{
	int n;
	ifstream file("default/index.html");
	ostringstream text;
	text << file.rdbuf();
	
	std::string response1 =
		"\n\r" +
		text.str();
	char buffer[4096];
	bzero(buffer, 4096);
	n = read(sock, buffer, 4096);
	// Test
	Headers header;
	header += string(buffer);
	map<string, string>oui = header.last();
	int status_code = 0;
	status_code = header.check(oui);
	cout << "\e[1;"<< 32 + _id <<"m [" << _id << "] Request #" << _count_requests++ << " " << oui.find("Request-Type")->second;
	if (oui.find("Request-Type")->second == "POST")
		cout << " | [" << oui.find("Content-Length")->second << "].\e[0;0m\n";
	else
		cout << "\e[0;0m\n";
	// if (status_code != STATUS_OK)
	// {
	// 	//  string response = header.return_response_header(status_code, header);
	// 	// n = write(sock, response.c_str(), strlen(response.c_str()));
	// 	return ;
	// }
	if (!dispatcher_type_requests(oui))
		;
	 string response = header.return_response_header(status_code, header, text.str().size());
	 response += response1;
	// for(map<string, string>::iterator it = oui.begin(); it != oui.end(); it++)
	// 	dprintf(1,"\e[92m%s\e[0m -> |\e[93m%s\e[0m|\n", it->first.c_str(), it->second.c_str());
	std::string new_buffer(buffer);
	
	// Fin de test;
	if (n < 0)
		error("ERROR reading from socket");
	//printf("\e[95mHere is the message:\n%s\e[0m\n", buffer);
	n = send(sock, response.c_str(), strlen(response.c_str()), MSG_DONTWAIT);
	// n = write(sock, response.c_str(), strlen(response.c_str()));
		// dprintf(1, "combien tu as print mon coquin ? %d %lu\n", n, strlen(response.c_str()));
	// cout << "HEADER RENVOYEE:\n" << response << "\e[0m" << endl;
	if (n < 0)
		error("ERROR writing to socket_nbet");
}

void Server::error(const char *s)
{
	perror(s);
	exit(1);
}