#include "Server.hpp"
#include "Utils.hpp"
#include "Client.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

Server::Server(int port)
{
	//Socket()
	_socket = socket(AF_INET, SOCK_STREAM, 0);
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
}

Server::Server(_t_preServ pre, pthread_mutex_t *logger)
{
	// Init thread
	_id = pre.id;
	_logger = logger;

	log("\e[33;1m[Starting ...]");
	thread = new pthread_t;
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
		perror("ERROR opening socket");

	//IOCTL()
	int opt = 1;
	setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
	// ioctl(_socket, FIONBIO, (char *)&opt);
	fcntl(_socket, F_SETFL, O_NONBLOCK);
	//BIND()
	bzero((char *)&_serv_addr, sizeof(_serv_addr));
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = INADDR_ANY;
	_serv_addr.sin_port = htons(pre.port);
	if (bind(_socket, (struct sockaddr *)&_serv_addr,
			 sizeof(_serv_addr)))
		perror("ERROR on binding");

	listen(_socket, 4096);

	FD_ZERO(&_clients_fd);
	_max_sd = _socket;
	FD_SET(_socket, &_clients_fd);

	_name = pre.name;
	_error_pages = pre.err;
	_root = pre._root;
}

Server::~Server()
{
	close(_socket);
}

Server &Server::operator=(Server const &ref)
{

	_serv_addr = ref._serv_addr;
	_cli_addr = ref._cli_addr;

	_id = ref._id;
	_socket = ref._socket;

	_name = ref._name;
	_root = ref._root;

	_error_pages = ref._error_pages;

	_root = ref._root;

	return *this;
}

// Hearth of the request for the server
void Server::run(map<int, Worker *> &workers)
{
	(void)workers;
	struct timeval tv = {100, 0};

	// cout << "Server: " << _id << endl;
	// getFile("/test/");
	fd_set temp_fd;
	log("\e[32;1m[Started !]");
	while (1)
	{

		int retval = 0;

		memcpy(&temp_fd, &_clients_fd, sizeof(_clients_fd));
		//temp_fd = _clients_fd;
		_desc_ready = 0;
		if ((retval = select(_max_sd + 1, &temp_fd, NULL, NULL, &tv)) > 0)
		{
			if (retval == -1)
				error("select crash");
			_desc_ready = 1;
			// _max_sd = _socket;
			bool connection_closed = false;
			bool server_end = false;
			// socklen_t clilen = sizeof(getCliAddr());

			// pthread_mutex_lock(getLogger());
			// // cout << "\e[1;96m[Worker " << to_string(w->getId()) << "]\e[0m";
			// pthread_mutex_unlock(getLogger());

			for (int fd = 1; fd <= _max_sd && _desc_ready > 0; fd++)
			{
				// cout << "je boucle ici " << fd << " == " << FD_ISSET(fd, &temp_fd) << "\n";
				if (FD_ISSET(fd, &temp_fd))
				{
					_desc_ready -= 1;
					if (fd == _socket)
					{

						int new_socket = 0;
						do
						{
							new_socket = accept(_socket, NULL, NULL);
							if (new_socket < 0)
							{
								if (errno != EWOULDBLOCK)
								{
									perror("Accept failed");
									server_end = true;
								}
								// cout << "oh la con de am tamere\n";
								break;
							}
							log("\e[1;94m[New incoming connection socket(" + to_string(fd) + ")]\e[0;0m");
							FD_SET(new_socket, &_clients_fd);
							if (new_socket > _max_sd)
								_max_sd = new_socket;

							// cout << " gros fil de pute : " << FD_ISSET(130, &_clients_fd) << " et lature " << new_socket <<  " et le fd " << fd << endl;
						} while (new_socket != -1);
						// cout << "oh la con de am tamere\n";
					}
					else
					{
						fcntl(fd, F_SETFL, O_NONBLOCK);
						Client client(getClients().size(), fd);
						connection_closed = false;
						int nbytes_read;
						string buff;
						char buffer[4096];
						bzero(buffer, 4096);
						do
						{
							int len_before_recv = sizeof(buffer);
							usleep(50);
							nbytes_read = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
							log("\e[1;93m[recv() read " + to_string(nbytes_read) + " characters]\e[0;0m");
							cout << buffer << endl;
							buff += buffer;

							bzero(buffer, 4096);
							client.setContent(client.getContent() + string(buffer));
							if (nbytes_read < 1 && nbytes_read < len_before_recv)
							{
								cout << buff << endl;
								connection_closed = true;
								// nbytes_read = send(fd, buffer, strlen(buffer), MSG_DONTWAIT);
								// log("\e[1;93m[send() wrote " + to_string(nbytes_read) + " characters]\e[0;0m");
								
								// if (nbytes_read < 0)
								// {
								// 	perror("error dans le sud\n");
								// 	server_end = true;

								// 	break;
								// }
								cout << nbytes_read << endl;
								// cout << (int)buffer[strlen(buffer) + 1] << " " << (int)buffer[strlen(buffer) - 1] << " " << (int)buffer[strlen(buffer) - 2] << " " << (int)buffer[strlen(buffer) -3] << " " <<(int) buffer[strlen(buffer)- 4] << endl;
								 //if (strncmp(buffer + (strlen(buffer) - 3), "\r\n", 2))
								 //	continue ;
								//cout << (int)buffer[strlen(buffer) + 1] << " " << (int)buffer[strlen(buffer) - 1] << " " << (int)buffer[strlen(buffer) - 2] << " " << (int)buffer[strlen(buffer) -3] << " " <<(int) buffer[strlen(buffer)- 4] << endl;
								client.setContent(buff);
								handle_request(client);
							
								break;
							}
							if (nbytes_read < 0)
							{
								if (errno != EWOULDBLOCK)
								{
									perror("recv() failed\n");
									server_end = true;
								}
								break;
							}
							// nbytes_read = send(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
							// if (nbytes_read < 0)
							// {
							// 	perror("error dans le sud\n");
							// 	server_end = true;
							// 	break;
							// }
						
						} while (true);
						//cout << "oui connection clsoed = " << connection_closed << endl;;
						if (connection_closed)
						{
							//Headers test;
							//test += buff;
							log("\e[1;31m[Connection closed]\e[0m");
							//cout << "\e[31m JE CLOSE \e[0m" << endl;
						//	if(!test.last().count("Connection") || test.last().find("Connection")->second.find("keep-alive"))
								close(fd);
							FD_CLR(fd, &_clients_fd);
							if (fd == _max_sd)
							{
								while (!FD_ISSET(_max_sd, &_clients_fd))
									_max_sd -= 1;
							}
						}
					}
				}
				// else
				// 	cout << "oh mama jai pas trouve le fd la...\n";
			}
		}

		else
			log("\e[1;96m[IDLING]\e[0m");
	}

	// // signal(SIGINT, this::signal_handle);
	// struct timeval tv = {1, 0};

	// _count_requests = 0;
	// int count = workers.size();
	// _workers = workers;
	// int idle = 0;

	// socklen_t clilen = sizeof(_cli_addr);

	// while (1)
	// {
	// 	fd_set write_fds2 = _write_fds;
	// 	fd_set read_fds2 = _read_fds;
	// 	// for (int l = 0; l < count; l++)
	// 	// 	workers.find(l)->second->setSocket(0);
	// 	if (select(_socket + 1, &read_fds2, &write_fds2, NULL, &tv) > 0)
	// 	{
	// 		// Search for an available worker
	// 		int i = 0;
	// 		while (1)
	// 		{
	// 			i = 0;
	// 			while (!workers.find(i)->second->getStatus() && i < count - 1)
	// 				i++;
	// 			if (workers.find(i)->second->getStatus())
	// 				break;
	// 			cout << "jboucle inf ici mais ca a pas de sens c si la\n";
	// 		}
	// 		if (!FD_ISSET(_socket, &_write_fds) || !FD_ISSET(_socket, &_read_fds))
	// 			error("ERROR non-set socket");
	// 		int newsockfd = accept(_socket, (struct sockaddr *)&_cli_addr, &clilen);
	// 		if (newsockfd < 0)
	// 			error("ERROR on accept");
	// 		cout << "boucle inf ici ma caille4!\n";
	// 		// Check for a existing client or a new one
	// 		if (!exists(newsockfd, clients))
	// 			error("Error during reading client socket part");
	// 		cout << "boucle inf ici ma caille5!\n";
	// 		workers.find(i)->second->setServer(this);
	// 		workers.find(i)->second->setStatus(false);
	// 		workers.find(i)->second->setSocket(newsockfd);
	// 		cout << "boucle inf ici ma caille523!\n";
	// 		// handle_request(newsockfd);
	// 		while (!workers.find(i)->second->getStatus())
	// 			;
	// 		cout << "boucle inf ici ma caille512!\n";
	// 		close(newsockfd);
	// 	}
	// 	else if (idle == 30)
	// 	{
	// 		log("\e[1;96m[IDLING]\e[0m");
	// 		idle = 0;
	// 	}
	// 	else
	// 		idle++;
	// }
}

t_loc *Server::findInLoc(string s, t_loc *root)
{
	for (list<t_loc *>::iterator it = root->childs.begin(); it != root->childs.end(); it++)
		if ((*it)->path == s)
			return (*it);
	return (NULL);
}

t_find Server::findAllLoc(string path)
{
	size_t pos;

	string split;

	t_loc *tmp = _root;

	t_find founded = {_root, ""};

	while ((pos = path.find("/")) != string::npos)
	{
		if (path[0] == '/')
		{
			founded.path += "/";
			path.erase(0, 1);
			continue;
		}
		split = path.substr(0, pos);
		tmp = findInLoc(split, tmp);
		if (!tmp)
			return founded;
		founded.path += split + "/";
		founded.loc = tmp;
		path.erase(0, pos + 1);
	}
	tmp = findInLoc(path, tmp);
	if (!tmp)
		return founded;
	founded.path += path;
	founded.loc = tmp;
	return founded;
}

t_file Server::getFile(string path)
{
	t_file file = {"", -1};
	t_find founded = findAllLoc(path);
	int fd;

//	cout << "founded:  " << founded.path << endl;

	size_t pos = founded.path.size();
	if (pos == 1)
		pos--;

	path.erase(0, pos);

	//cout << "path sub: " << path << endl;
	//cout << "root:     " << founded.loc->options.params.find("root")->second << endl;

	path = founded.loc->options.params.find("root")->second + path;
	//cout << "final:    " << path << endl;

	//Rajouter if auto-index
	if ((fd = open(path.c_str(), O_RDONLY | O_NONBLOCK | O_DIRECTORY)) != -1)
	{
		if (!founded.loc->options.params.count("index"))
		{
			cout << "Directory !" << endl;
			close(fd);
			return file;
		}
		else
			path += founded.loc->options.params.find("index")->second;
		close(fd);
	}
	//cout << "2:    " << path << endl;
	if ((fd = open(path.c_str(), O_RDONLY | O_NONBLOCK)) == -1)
	{
		cout << "Not Found !" << endl;
		return file;
	}
	cout << "Found !" << endl;
	char c;
	file.content.clear();
	while (read(fd, &c, 1))
		file.content.push_back(c);
	file.size = file.content.size();
	close(fd);
	return file;
}

void Server::handle_request(Client &client)
{
	// cout << "fdp de ta mere\n";
	string type[] = {"GET", "POST", "HEAD", "PUT"};
	string (Server::*command[])(map<string, string>, int) = {&Server::GET, &Server::POST, &Server::HEAD, &Server::PUT};
	string buffer = client.getContent();
	int n;
	// Read until header until is finished
	// bzero(buffer, 4096);
	// // int n = read(client->getSocket(), buffer, 4096);
	// cout << "Request :" << endl
	// 	 << buffer << endl
	// 	 << "=====" << endl;
	// cout << "allo1\n";
	Headers request;
	request += buffer;
	string response;
	map<string, string> p_request = request.last();

	string req_type = p_request.find("Request-Type")->second;
	cout << "allo2\n";
	for (size_t i = 0; i <= type->size(); i++)
	{
		if (i == type->size())
		{
			response = SEND_ERROR(STATUS_BAD_REQUEST, "Bad Request");
			break;
		}
		if (!strcmp(req_type.c_str(), type[i].c_str()))
		{

			if (check_methods(findAllLoc(p_request.find("Location")->second).loc, req_type))
				response = (this->*command[i])(p_request, client.getSocket());
			else
			{
				log("\e[1;93mMethod Not Allowed!\e[0m");
				response = SEND_ERROR(STATUS_METHOD_NOT_ALLOWED, "Method Not Allowed");
			}
			break;
		}
	}
	// cout << "allo3\n";
	cout << endl
		 << "La response ici : \n"
		 << response << endl
		 << endl;
	cout << client.getSocket() << " et la taille : " << client.getContent().size() << endl;
	cout << "\e[31m JE WRITE \e[0m";
	n = send(client.getSocket(), response.c_str(), response.size(), MSG_DONTWAIT);
	//n = write(sock, response.c_str(), strlen(response.c_str()));
	// cout << "allo4\n";
	if (n < 0)
		error("Can't send the response");
	// close(client.getSocket());
}

void Server::error(const char *s)
{
	perror(s);
	exit(1);
}

//PAS FINIS
string Server::POST(map<string, string> header, int socket)
{

	log("\e[1;93m[POST -> " + header.find("Location")->second + "]\e[0m");

	(void)socket;
	Headers tmp;
	string resp, content;

	// A patch des que getfile marche
	string path = "default" + header.find("Location")->second;
	//struct stat sb;

	const char *content_char = header.find("Content")->second.c_str();
	if (header.find("Content-Length")->second == "0")
	{
		return (tmp.return_response_header(STATUS_NO_CONTENT, tmp, 0));
	}
	// cout << content_char << endl;

	const char *to_print = content_char;

	t_file file = getFile(header.find("Location")->second);
	if (header.count("Content") && !header.find("Content")->second.size())
		resp = SEND_ERROR(STATUS_NO_CONTENT, "No Content");

	// A UPDATE tres rapidement des que le GetFile est patch
	cout << "ici1\n";
	// cout << " je print :" << to_print << ":\n";
	int nb_prints, fd = open("default/file.txt", O_TRUNC | O_WRONLY | O_NONBLOCK, 0777);
	cout << "je crash ici\n";
	// istringstream iss(header.find("Content-Length")->second);
	cout << "je crash ici\n";
	size_t remaining_characters, count = 0;
	remaining_characters = header.find("Content")->second.size();
	while (count < remaining_characters)
	{
		if (65535 < count)
			nb_prints = 65535;
		else
		{
			to_print = content_char + count;
			nb_prints = strlen(content_char);
		}
		// cout << "nb_prints = " << nb_prints << " toPrint = " << to_print << endl;
		count += write(fd, to_print, nb_prints);
		// cout <<  count << endl;
	}
	cout << "ici2\n";
	close(fd);

	resp = tmp.return_response_header(200, tmp, file.size);
	return resp;
}

string Server::PUT(map<string, string> header, int socket)
{
	(void)socket;
	(void)header;
	return NULL;
}

// A ameliorer plus tard
string Server::HEAD(map<string, string> header, int socket)
{
	(void)socket;
	Headers tmp;
	string resp;
	t_file file = getFile(header.find("Location")->second);

	// Return only sizeof file size - content size
	resp = tmp.return_response_header(200, tmp, 0);
	resp += "Location: http://localhost:8080/\r\n\r\n";
	cout << resp;
	return resp;
}

//A AMELIORER plus tard
string Server::GET(map<string, string> header, int socket)
{
	cout << "je veux sortir sac a merde1\n";
	(void)socket;
	log("\e[1;93m[GET -> " + header.find("Location")->second + "]\e[0m");

	t_file file = getFile(header.find("Location")->second);
	Headers tmp;
	string resp;



	if (file.size == -1)
		resp = SEND_ERROR(STATUS_NOT_FOUND, "Not Found");
	else if (header.count("coffee"))
		resp = SEND_ERROR(STATUS_TEAPOT, "I'm a teapot");
	else
	{
		resp = tmp.return_response_header(200, tmp, 0);
		resp += file.content;
	}
	cout << "je veux sortir sac a merde\n";
	return resp;
}

// SOUCIS: si on  location /foo/ et location /foo/bar/ dans le .conf et aue l'url est /foo/bar/, il va chopper /foo/
// il faut donc le comparer avec tout et prendre celui avec le + de char correspondant

// Generate a default error page -- TODO: add conf errors page
string Server::SEND_ERROR(int status, const char *msg)
{
	t_file file;
	string resp;
	Headers tmp;

	char c;
	size_t pos;

	int fd = open("default/error.html", O_RDONLY | O_NONBLOCK);
	while (read(fd, &c, 1))
		file.content.push_back(c);
	while ((pos = file.content.find("_CODE_")) != file.content.npos)
		file.content.replace(file.content.find("_CODE_"), 6, to_string(status));
	while ((pos = file.content.find("_MSG_")) != file.content.npos)
		file.content.replace(file.content.find("_MSG_"), 5, msg);
	file.size = file.content.size();
	resp = tmp.return_response_header(status, tmp, file.size);
	resp += file.content;
	close(fd);
	return resp;
}

void Server::log(string s)
{
	struct timeval tv;
	time_t t;
	struct tm *info;
	char buffer[64];
	pthread_mutex_lock(_logger);

	gettimeofday(&tv, NULL);
	t = tv.tv_sec;

	info = localtime(&t);
	strftime(buffer, sizeof buffer, "[%c]", info);
	cout << "\e[1;" << to_string(93 + _id) << "m[SERVER " << _id << "]\e[0m" << s << " \e[1;90m" << buffer << "\e[0m" << endl;

	pthread_mutex_unlock(_logger);
}

bool Server::check_methods(t_loc *root, string meth)
{
	for (vector<string>::iterator it = root->options.methods.begin(); it != root->options.methods.end(); it++)
		if (*it == meth)
			return true;
	return false;
}

// Check if a client already exists with this socket
int Server::exists(int socket, list<Client> &clients)
{
	bool found = false;
	list<Client>::iterator begin = clients.begin();
	while (begin != clients.end())
	{
		if (begin->getSocket() == socket)
		{
			if (!(begin->readContent()))
				return (false);
			found = true;
		}
		cout << "boucle inf ici ma caille!2\n";
		begin++;
	}
	cout << "boucle inf ici ma caille!6" << found << "\n";
	if (!found)
	{
		Client client(clients.size() + 1, socket);
		cout << "avant read ta mere la cheinne\n";
		if (!(client.readContent()))
			return (false);
		cout << "apres read ta mere la cheinne\n";
		clients.push_back(client);
	}
	cout << "boucle inf ici ma caille!7\n";
	return true;
}

int Server::findAvailableWorker(map<int, Worker *> &workers)
{
	size_t i = 0;
	while (1)
	{
		i = 0;
		while (!workers.find(i)->second->getStatus() && i < workers.size() - 1)
			i++;
		if (workers.find(i)->second->getStatus())
			break;
	}
	return (i);
}