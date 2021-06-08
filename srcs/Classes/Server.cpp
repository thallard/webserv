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

Server::Server(_t_preServ pre, pthread_mutex_t *logger, map<string, string>mimes)
{
	// Init thread
	_id = pre.id;
	_logger = logger;

	_port = pre.port;

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
	_extensions = pre.ext;
	_auto_index = pre.auto_index;
	_mimes = mimes;
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
	_auto_index = ref._auto_index;
	_port = ref._port;
	_extensions = ref._extensions;
	_mimes = ref._mimes;
	return *this;
}

// Hearth of the request for the server
void Server::run(map<int, Worker *> &workers)
{
	(void)workers;
	struct timeval tv = {100, 0};

	// std::cout << "Server: " << _id << endl;
	// getFile("/test/");
	fd_set temp_fd;
	log("\e[32;1m[Started !]");
	while (1)
	{

		int retval = 0, index = 0;

		memcpy(&temp_fd, &_clients_fd, sizeof(_clients_fd));
		//temp_fd = _clients_fd;
		_desc_ready = 0;
		if ((retval = select(_max_sd + 1, &temp_fd, NULL, NULL, &tv)) >= 1)
		{
			if (retval == -1)
				error("select crash");
			_desc_ready = 1;
			// _max_sd = _socket;
			// bool connection_closed = false;
			// bool server_end = false;
			setTempFD(temp_fd);
			// setDescReady(1);
			// setMaxSD(_max_sd);
			// dprintf(1, "socket %d\n", _max_sd);
			index = findAvailableWorker(workers);
			workers.find(index)->second->setServer(this);
			workers.find(index)->second->setIt(0);
			workers.find(index)->second->setStatus(false);

			workers.find(index)->second->setSocket(_max_sd);
			while (!workers.find(index)->second->getStatus())
				;
			workers.find(index)->second->setIt(0);
			// socklen_t clilen = sizeof(getCliAddr());

			// pthread_mutex_lock(getLogger());
			// // std::cout << "\e[1;96m[Worker " << to_string(w->getId()) << "]\e[0m";
			// pthread_mutex_unlock(getLogger());

			// for (int fd = 1; fd <= _max_sd && _desc_ready > 0; fd++)
			// {
			// 	// std::cout << "je boucle ici " << fd << " == " << FD_ISSET(fd, &temp_fd) << "\n";
			// 	if (FD_ISSET(fd, &temp_fd))
			// 	{
			// 		_desc_ready -= 1;
			// 		if (fd == _socket)
			// 		{

			// 			int new_socket = 0;
			// 			do
			// 			{
			// 				new_socket = accept(_socket, NULL, NULL);
			// 				if (new_socket < 0)
			// 				{
			// 					if (errno != EWOULDBLOCK)
			// 					{
			// 						perror("Accept failed");
			// 						server_end = true;
			// 					}
			// 					// std::cout << "oh la con de am tamere\n";
			// 					break;
			// 				}
			// 				log("\e[1;94m[New incoming connection socket(" + to_string(fd) + ")]\e[0;0m");
			// 				FD_SET(new_socket, &_clients_fd);
			// 				if (new_socket > _max_sd)
			// 					_max_sd = new_socket;

			// 				// std::cout << " gros fil de pute : " << FD_ISSET(130, &_clients_fd) << " et lature " << new_socket <<  " et le fd " << fd << endl;
			// 			} while (new_socket != -1);
			// 			// std::cout << "oh la con de am tamere\n";
			// 		}
			// 		else
			// 		{
			// 			fcntl(fd, F_SETFL, O_NONBLOCK);
			// 			Client client(getClients().size(), fd);
			// 			connection_closed = false;
			// 			int nbytes_read;
			// 			string buff;
			// 			char buffer[4096];
			// 			bzero(buffer, 4096);
			// 			do
			// 			{
			// 				int len_before_recv = sizeof(buffer);
			// 				usleep(50);
			// 				nbytes_read = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
			// 				log("\e[1;93m[recv() read " + to_string(nbytes_read) + " characters]\e[0;0m");
			// 				std::cout << buffer << endl;
			// 				buff += buffer;

			// 				bzero(buffer, 4096);
			// 				client.setContent(client.getContent() + string(buffer));
			// 				if (nbytes_read < 1 && nbytes_read < len_before_recv)
			// 				{
			// 					std::cout << buff << endl;
			// 					connection_closed = true;
			// 					// nbytes_read = send(fd, buffer, strlen(buffer), MSG_DONTWAIT);
			// 					// log("\e[1;93m[send() wrote " + to_string(nbytes_read) + " characters]\e[0;0m");

			// 					// if (nbytes_read < 0)
			// 					// {
			// 					// 	perror("error dans le sud\n");
			// 					// 	server_end = true;

			// 					// 	break;
			// 					// }
			// 					std::cout << nbytes_read << endl;
			// 					// std::cout << (int)buffer[strlen(buffer) + 1] << " " << (int)buffer[strlen(buffer) - 1] << " " << (int)buffer[strlen(buffer) - 2] << " " << (int)buffer[strlen(buffer) -3] << " " <<(int) buffer[strlen(buffer)- 4] << endl;
			// 					//if (strncmp(buffer + (strlen(buffer) - 3), "\r\n", 2))
			// 					//	continue ;
			// 					//std::cout << (int)buffer[strlen(buffer) + 1] << " " << (int)buffer[strlen(buffer) - 1] << " " << (int)buffer[strlen(buffer) - 2] << " " << (int)buffer[strlen(buffer) -3] << " " <<(int) buffer[strlen(buffer)- 4] << endl;
			// 					client.setContent(buff);
			// 					handle_request(client);

			// 					break;
			// 				}
			// 				if (nbytes_read < 0)
			// 				{
			// 					if (errno != EWOULDBLOCK)
			// 					{
			// 						perror("recv() failed\n");
			// 						server_end = true;
			// 					}
			// 					break;
			// 				}
			// 				// nbytes_read = send(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
			// 				// if (nbytes_read < 0)
			// 				// {
			// 				// 	perror("error dans le sud\n");
			// 				// 	server_end = true;
			// 				// 	break;
			// 				// }

			// 			} while (true);
			// 			//std::cout << "oui connection clsoed = " << connection_closed << endl;;
			// 			if (connection_closed)
			// 			{
			// 				//Headers test;
			// 				//test += buff;
			// 				log("\e[1;31m[Connection closed]\e[0m");
			// 				//std::cout << "\e[31m JE CLOSE \e[0m" << endl;
			// 				//	if(!test.last().count("Connection") || test.last().find("Connection")->second.find("keep-alive"))
			// 				close(fd);
			// 				FD_CLR(fd, &_clients_fd);
			// 				if (fd == _max_sd)
			// 				{
			// 					while (!FD_ISSET(_max_sd, &_clients_fd))
			// 						_max_sd -= 1;
			// 				}
			// 			}
			// 		}
			// 	}
			// else
			// 	std::cout << "oh mama jai pas trouve le fd la...\n";
		}
		else
			log("\e[1;96m[IDLING]\e[0m");
	}
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

	//	std::cout << "founded:  " << founded.path << endl;

	size_t pos = founded.path.size();
	if (pos == 1)
		pos--;

	string a_i = path;
	path.erase(0, pos);

	//std::cout << "path sub: " << path << endl;
	//std::cout << "root:     " << founded.loc->options.params.find("root")->second << endl;

	path = founded.loc->options.params.find("root")->second + path;
	//std::cout << "final:    " << path << endl;

	//Rajouter if auto-index
	if ((fd = open(path.c_str(), O_RDONLY | O_NONBLOCK | O_DIRECTORY)) != -1)
	{
		if (!_auto_index)
		{
			if (!founded.loc->options.params.count("index"))
			{
				std::cout << "Directory !" << endl;
				close(fd);
				return file;
			}
			else
				path += founded.loc->options.params.find("index")->second;
		}
		else
		{
			file.content = "<html> <head><title>Index of " + a_i + "</title></head> <body bgcolor=\"white\"> <h1>Index of "+ a_i +"</h1><hr><pre>\n";
			
			DIR *dir;
			DIR *dir_tmp;

			struct dirent *ent;
			size_t size;
			size_t size_2;
			if ((dir_tmp = opendir (path.c_str())) != NULL)
			{
				size_2 = 0;
				while ((ent = readdir (dir_tmp)) != NULL)
				{
					struct stat buf;
					bzero(&buf, sizeof(buf));
					stat(string(path+ "/" + string(ent->d_name)).c_str(), &buf);
					string size_file = to_string(buf.st_size);
					if (size_2 < size_file.size())
						size_2 = size_file.size();
				}
			}
			if ((dir = opendir (path.c_str())) != NULL)
			{
				size = file.content.size();
				while ((ent = readdir (dir)) != NULL)
				{
					size = string(ent->d_name).size();
					if (ent->d_type == DT_DIR)
					{
						size++;
						file.content += "<a href=\"" + string(ent->d_name) + "/\">" + string(ent->d_name) + "/</a>";
					}
					else
						file.content += "<a href=\"" + string(ent->d_name) + "\">" + string(ent->d_name) + "</a>";
					if (string(ent->d_name) != ".." && string(ent->d_name) != ".")
					{
						while (size++ < 51)
							file.content += " ";
						struct stat buf;
						bzero(&buf, sizeof(buf));
						stat(string(path+ "/" + string(ent->d_name)).c_str(), &buf);

					struct timeval tv;
					time_t t;
					struct tm *info;
					char buffer[64];

					gettimeofday(&tv, NULL);
					t = tv.tv_sec;

					info = localtime(&(buf.st_mtime));
					strftime(buffer, sizeof(buffer), "%d-%b-%Y %H:%M", info);
						file.content += buffer;
						size_t i = 0;
						while (i++ < 20 - size_2)
							file.content += " ";
						string size_file = to_string(buf.st_size);
						i = size_file.size();
						while (i++ != size_2)
							file.content += " ";
						file.content += size_file;
						
					}
					file.content += "\n";
				}
				
				closedir (dir);
			}
			file.content += "</pre><hr></body> </html> ";
			file.size = file.content.size();
			close(fd);
			return file;
		}
		close(fd);
	}
	//std::cout << "2:    " << path << endl;
	if ((fd = open(path.c_str(), O_RDONLY | O_NONBLOCK)) == -1)
	{
		std::cout << "Not Found !" << endl;
		return file;
	}
	std::cout << "Found !" << endl;
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
	string type[4] = {"GET", "POST", "HEAD", "PUT"};
	string (Server::*command[])(map<string, string>, Client &) = {&Server::GET, &Server::POST, &Server::HEAD, &Server::PUT};
	string buffer = client.getContent();
	int n;

	Headers request;
	request += buffer;
	map<string, string> p_request = request.last();
	string response, req_type = p_request.find("Request-Type")->second;

	std::cout << "Request :" << endl
			  << buffer << endl
			  << "=====" << endl;

	for (size_t i = 0; i <= type->size(); i++)
	{
		if (!strncmp(req_type.c_str(), type[i].c_str(), req_type.size()))
		{
			if (check_methods(findAllLoc(p_request.find("Location")->second).loc, req_type, p_request.find("Location")->second))
				response = (this->*command[i])(p_request, client);
			else
				response = SEND_ERROR(STATUS_METHOD_NOT_ALLOWED, "Method Not Allowed");
			break;
		}
		else if (i == type->size())
		{
			response = SEND_ERROR(STATUS_BAD_REQUEST, "Bad Request");
			break;
		}
	}

	// DEBUG
	std::cout << endl
			  << "La response ici : \n"
			  << response << endl
			  << endl;

	n = send(client.getSocket(), response.c_str(), response.size(), 0);
	if (n < 0)
		error("Can't send the response");
}

void Server::error(const char *s)
{
	perror(s);
	exit(1);
}

//PAS FINIS
string Server::POST(map<string, string> header, Client &client)
{
	Headers tmp;
	string
		resp,
		content = header.find("Content")->second.c_str();
	int nb_print = 0;
	char buffer[65535];
	ofstream ofile;

	string loc = header.find("Location")->second;

	log("\e[1;93m[POST -> " + header.find("Location")->second + "]\e[0m");
	string extension;
	for (string::reverse_iterator it = loc.rbegin(); it != loc.rend(); it++)
	{
		extension.insert(extension.begin(), *it);
		if (*it == '.')
			break;
	}
	if (_extensions.count(extension) && _extensions.find(extension)->second.count("allow_methods"))
	{
	}
	client.setPath(loc);
	cout << "La location : " << loc << endl;
	// size_t pos;
	//if ((pos = loc.rfind(".bla", loc.size() - 4)) != loc.npos)
	if (_extensions.count(extension) && _extensions.find(extension)->second.count("cgi_path"))
	{
		CGI *cgi = new CGI(*this, header, client);
		(void)cgi;
		// (void)cgi;
		// cout << "j'ai un .bla a la fin batard" << endl;
		// // CGI cgi();
		// char *env[15];
		// env[0] = (char *)"REMOTE_HOST=127.0.0.1";
		// env[1] = (char *)"SERVER_NAME=webserv";
		// env[2] = (char *)"SERVER_PORT=8080";
		// env[3] = (char *)"SERVER_PROTOCOL=HTTP/1.1";
		// env[4] = (char *)"SERVER_SOFTWARE=HTTP/1.1";
		// env[5] = (char *)"REQUEST_METHOD=POST";
		// env[6] = (char *)"PATH_INFO=/Users/thallard/Documents/42/webserv/YoupiBanane/youpi.bla";
		// // env[7] = (char *)"SCRIPT_NAME=/Users/thallard/Documents/42/webserv";
		// env[7] = (char *)NULL;
		// char *cgii[3] = {(char *)_extensions.find(extension)->second.find("cgi_path")->second.at(0).c_str(), NULL};
		// pid_t forke;
		// int ret = 0;
		// // int fd = 0;
		// int pipe1[2];
		// int pipe2[2];
		// if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
		// {
		// 	perror("Pipe failed");
		// 	exit(EXIT_FAILURE);
		// }
		// forke = fork();
		// if (forke < 0)
		// {
		// 	perror("Fork failed");
		// 	exit(EXIT_FAILURE);
		// }
		// if (forke == 0)
		// {
		// 	// fd = open("default/file.txt", 0777);
		// 	dup2(pipe1[0], STDIN_FILENO);
		// 	close(pipe1[0]);
		// 	close(pipe1[1]);
		// 	dup2(pipe2[1], STDOUT_FILENO);
		// 	close(pipe2[0]);
		// 	close(pipe2[1]);
		// 	ret = execve(cgii[0], cgii, env);
		// 	int errnum = errno;
		// 	fprintf(stderr, "Failed to execute '%s' (%d: %s)\n", "tqt ", errnum, strerror(errnum));
		// 	exit(EXIT_FAILURE);
		// }
		// else
		// {
		// 	close(pipe1[0]);
		// 	// write(1, client.getContent().c_str(), client.getContent().size());
		// 	int oui = write(pipe1[1], client.getContent().c_str(), client.getContent().size());
		// 	// close(pipe1[1]);
		// 	// client.setSocket(pipe1[1]);
		// 	// readPerChunks(client, "POST", header);
		// 	if (oui <= 0)
		// 	{
		// 		perror("Failed to write from pipe");
		// 		exit(EXIT_FAILURE);
		// 	}
		// 	close(pipe1[1]);
		// 	char buf[1550];

		// 	int nb = read(pipe2[0], &buf, 1000);
		// 	if (nb <= 0)
		// 	{
		// 		perror("Failed to read from pipe");
		// 		exit(EXIT_FAILURE);
		// 	}
		// 	dprintf(1, "le buf : [%s]\n", buf);
		// 	close(pipe2[0]);
		// 	// close(fd);
		// 	 waitpid(forke, NULL, 1);
		// }
		// cout << errno << endl;
		// cout << strerror(errno) << endl;
		// cout << ret << endl;
		// exit(1);
	}

	// Check if the content is chunked
	if (header.count("Transfer-Encoding") && !strncmp(header.find("Transfer-Encoding")->second.c_str(), "chunked", 8))
	{
		// readPerChunks(client, "POST", header);

		resp = tmp.return_response_header(STATUS_OK, tmp, 0);
		return resp;
	}

	if (header.find("Content-Length")->second == "0")
		return (tmp.return_response_header(STATUS_NO_CONTENT, tmp, 0));

	t_file file = getFile(header.find("Location")->second);
	if (header.count("Content") && !header.find("Content")->second.size())
		resp = SEND_ERROR(STATUS_NO_CONTENT, "No Content");

	// Open the file and print content in
	ofile.open("default/file_post.txt", ios::app);

	while (nb_print < atoi(header.find("Content-Length")->second.c_str()))
	{
		bzero(buffer, 65535);

		nb_print += content.size();
		ofile << content;
		content.clear();

		recv(client.getSocket(), buffer, 65535, 0);
		content = buffer;
	}

	resp = tmp.return_response_header(200, tmp, file.size);
	return resp;
}

// PUT method (create new file or trunk existent content)
string Server::PUT(map<string, string> header, Client &client)
{
	(void)client;
	// Refactoring en cours
	log("\e[1;93m[PUT -> " + header.find("Location")->second + "]\e[0m");
	Headers tmp;
	ofstream ofile;
	string resp, content, path = header.find("Location")->second;

	ofile.open("./default/file_put.txt", ios::app);
	ofile << "bonjourewe\n";

	// ofile.open("default/file_put.txt", ofstream::out | ofstream::trunc);
	if (!strncmp(header.find("Transfer-Encoding")->second.c_str(), "chunked", 8))
	{
		readPerChunks(client, "PUT", header);
		resp = tmp.return_response_header(200, tmp, content.size());
		return resp;
		// resp += content;
		// content = readPerChunks(header, socket);
	}
	else
		content = header.find("Content")->second;
	ofile.close();

	const char *content_char = content.c_str();
	const char *to_print = content_char;

	t_file file = getFile(header.find("Location")->second);
	if (header.count("Content") && !header.find("Content")->second.size())
		resp = SEND_ERROR(STATUS_NO_CONTENT, "No Content");

	int nb_prints, fd = open("./default/file.txt", O_TRUNC | O_CREAT | O_WRONLY | O_NONBLOCK, 0777);
	size_t remaining_characters = content.size(), count = 0;
	while (count < remaining_characters)
	{
		if (65535 < count)
			nb_prints = 65535;
		else
		{
			to_print = content_char + count;
			nb_prints = strlen(content_char);
		}
		count += write(fd, to_print, nb_prints);
	}

	close(fd);

	resp = tmp.return_response_header(200, tmp, content.size());
	resp += content;
	return (resp);
}

// A ameliorer plus tard
string Server::HEAD(map<string, string> header, Client &client)
{
	(void)client;
	(void)socket;
	Headers tmp;
	string resp;
	t_file file = getFile(header.find("Location")->second);

	// Return only sizeof file size - content size
	resp = tmp.return_response_header(200, tmp, 0);
	resp += "Location: http://localhost:8080/\r\n\r\n";
	std::cout << resp;
	return resp;
}

//A AMELIORER plus tard
string Server::GET(map<string, string> header, Client &client)
{
	(void)client;
	std::cout << "je veux sortir sac a merde1\n";
	(void)socket;
	log("\e[1;93m[GET -> " + header.find("Location")->second + "]\e[0m");

	t_file file = getFile(header.find("Location")->second);
	Headers tmp;
	string resp;

	string extension;
	string loc = header.find("Location")->second;
	cout << "test" << endl;
	for (string::reverse_iterator it = loc.rbegin(); it != loc.rend(); it++)
	{
		if (*it == '.')
			break;
		extension.insert(extension.begin(), *it);
	}
cout << extension << endl;
	if (file.size == -1)
		resp = SEND_ERROR(STATUS_NOT_FOUND, "Not Found");
	else if (header.count("coffee"))
		resp = SEND_ERROR(STATUS_TEAPOT, "I'm a teapot");
	else
	{
		if (_mimes.count(extension))
			resp = tmp.return_response_header(200, tmp, 0, _mimes.find(extension)->second);
		else
			resp = tmp.return_response_header(200, tmp, 0);
		resp += file.content;
	}
	std::cout << "je veux sortir sac a merde\n";
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

	log("\e[1;31m[Method Not Allowed]\e[0;0m");

	string page;
	if (_error_pages.count(status))
		page = _error_pages.find(status)->second;
	else
		page = "default/error.html";
	cout << page << endl;
	int fd = open(page.c_str(), O_RDONLY | O_NONBLOCK);
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
	strftime(buffer, sizeof(buffer), "[%c]", info);
	std::cout << "\e[1;" << to_string(93 + _id) << "m[SERVER " << _id << "::" << _port << "]\e[0m" << s << " \e[1;90m" << buffer << "\e[0m" << endl;

	pthread_mutex_unlock(_logger);
}

bool Server::check_methods(t_loc *root, string meth, string loc)
{
	string extension;
	vector<string> allowed;
	for (string::reverse_iterator it = loc.rbegin(); it != loc.rend(); it++)
	{
		extension.insert(extension.begin(), *it);
		if (*it == '.')
			break;
	}
	cout << "extension: |" << extension << "|" << endl;
	if (_extensions.count(extension) && _extensions.find(extension)->second.count("allow_methods"))
	{
		cout << "found custom extension !" << endl;
		allowed = _extensions.find(extension)->second.find("allow_methods")->second;
	}
	else
		allowed = root->options.methods;

	for (vector<string>::iterator it = allowed.begin(); it != allowed.end(); it++)
	{
		cout << "allowed here: " << *it << endl;
		if (*it == meth)
			return true;
	}
	return false;
}

// Check if a client already exists with this socket
// int Server::exists(int socket, list<Client> &clients)
// {
// 	bool found = false;
// 	list<Client>::iterator begin = clients.begin();
// 	while (begin != clients.end())
// 	{
// 		if (begin->getSocket() == socket)
// 		{
// 			if (!(begin->readContent()))
// 				return (false);
// 			found = true;
// 		}
// 		std::cout << "boucle inf ici ma caille!2\n";
// 		begin++;
// 	}
// 	std::cout << "boucle inf ici ma caille!6" << found << "\n";
// 	if (!found)
// 	{
// 		Client client(clients.size() + 1, socket);
// 		std::cout << "avant read ta mere la cheinne\n";
// 		if (!(client.readContent()))
// 			return (false);
// 		std::cout << "apres read ta mere la cheinne\n";
// 		clients.push_back(client);
// 	}
// 	std::cout << "boucle inf ici ma caille!7\n";
// 	return true;
// }

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

string Server::readPerChunks(Client &client, string method, map<string, string> header)
{
	(void)header;
	static int total = 1;
	string content, temp;

	cout << "on rentre bien dans le read per chumnks\n";

	// cout << client.getContent() << endl;
	size_t pos = client.getContent().find("\r\n\r\n", 0);
	temp = &client.getContent().at(pos + 4);
	// cout << "[" << temp << "]" << endl;
	ofstream ofile;

	if (!strcmp("PUT", method.c_str()))
		ofile.open("./default/file_put.text", ios::out);
	else
		ofile.open("./default/file_post.text");

	int maxBody = -1;
	t_find file = findAllLoc("./default/file_post.text");
	if (file.loc->options.params.count("maxBody"))
		maxBody = atoi(file.loc->options.params.find("maxBody")->second.c_str());

	// Get the length of the chunk
	string length_char;
	int i = 0, length = 0;
	while (temp.at(i) != '\r' && temp.at(i) != '\n')
		length_char += temp.at(i++);

	// Remove hexadecimal characters and \r\n + transform hexadecimal length to decimal
	temp = &temp.at(i + 2);
	stringstream stream;
	stream << hex << length_char;
	stream >> length;
	// if (!strcmp("POST", method.c_str()))
	// {
	// 	temp[100] = '\0';
	// 	ofile << temp;
	// 	sleep(10);
	// 	ofile.close();
	// 	return content;
	// }
	if (!length)
		return content;
	cout << "Taille de la chunk : " << length << endl;
	// Append existent content from worker's recv
	for (int j = 0; j < length; j++)
		content += temp.at(j);
	temp = &temp.at(length + 2);
	cout << "Dernier charactere du temp : " << temp.size() << endl;
	ofile << content;
	char buff[length + 1];

	// int remaining_characters;
	while (true)
	{
		char buf[65535];
		bzero(buf, 65535);
		length_char.clear();
		i = 0;
		while (temp.at(i) != '\r' && temp.at(i) != '\n')
			length_char += temp.at(i++);
		temp = &temp.at(i + 2);

		stringstream stream;
		stream << hex << length_char;
		stream >> length;
		if (!length)
			break;
		// cout << length_char << endl;
		cout << "Taille de la chunk : " << length << " et la taille de temp : " << temp.size() << endl;
		int length_copy = 0;
		if (length > static_cast<int>(temp.size()))
		{
			ofile << temp;
			// content.copy((char *)temp.c_str(), )
			total += temp.size();
			length_copy = temp.size();
		}
		else
		{
			bzero(buff, length + 1);
			// content_print.clear();
			// temp[temp.size()] = '\0';
			temp.copy(buff, length, 0);
			// cout << "dans le else : " << strlen(buff) << endl;
			temp = &temp.at(length + 2);
			total += strlen(buff);
			ofile << buff;
			continue;
			// ofile.close();
			// break ;
		}

		int nbytes_read = 0;
		usleep(50);
		do
		{
			nbytes_read += recv(client.getSocket(), &buf[strlen(buf)], 65000 - strlen(buf), 0);
			// log("\e[1;93m[recv() read " + to_string(nbytes_read) + " characters]\e[0;0m");
			// cout << nbytes_read << endl;
			if (strstr(buf, "0\r\n\r\n"))
				break;
		} while (65000 - strlen(buf) > 1);

		temp.clear();
		temp = buf;
		bzero(buff, length + 1);
		temp.copy(buff, length - length_copy, 0);
		total += strlen(buff);
		ofile << buff;
		temp = &temp.at(length - length_copy + 2);
		content.clear();
	}
	cout << "Characteres print de mon cote : " << total << " vrai nombre que je devrais print : " << 10000000 << endl;
	content.clear();
	ofile.close();
	// string content, temp, length_char;
	// stringstream stream;
	// ofstream ofile;

	// temp = &client.getContent().at(client.getContent().find("\r\n\r\n", 0) + 4);

	// if (!strncmp(method.c_str(), "PUT", 3))
	// 	ofile.open("./default/file_put.text", ios::out);
	// else
	// 	ofile.open("./default/file_post.txt", ios::out | ios::app);

	// // Get the length of the chunk
	// int i = 0, length = 0;
	// while (temp.at(i) != '\r' && temp.at(i) != '\n')
	// 	length_char += temp.at(i++);

	// // Remove hexadecimal characters and \r\n from temp + transform hexadecimal length to decimal
	// temp = &temp.at(i + 2);
	// stream << hex << length_char;
	// stream >> length;

	// // Append existent content from worker's recv
	// for (int j = 0; j < length; j++)
	// 	content += temp.at(j);
	// temp = &temp.at(length + 2);
	// ofile << content;
	// char buff[length + 1];

	// while (true)
	// {
	// 	char buf[65535];
	// 	bzero(buf, 65535);
	// 	length_char.clear();
	// 	int length_copy = 0, i = 0, nbytes_read = 0;

	// 	while (temp.at(i) != '\r' && temp.at(i) != '\n')
	// 		length_char += temp.at(i++);
	// 	temp = &temp.at(i + 2);

	// stringstream stream;
	// 	stream << hex << length_char;
	// 	stream >> length;
	// 	if (!length)
	// 		break;
	// 	if (length > static_cast<int>(temp.size()))
	// 	{
	// 		ofile << temp;
	// 		length_copy = temp.size();
	// 	}
	// 	else
	// 	{
	// 		bzero(buff, length + 1);
	// 		temp.copy(buff, length, 0);
	// 		temp = &temp.at(length + 2);
	// 		ofile << buff;
	// 		continue;
	// 	}
	// 	usleep(50);
	// 	do
	// 	{
	// 		nbytes_read += recv(client.getSocket(), &buf[strlen(buf)], 65000 - strlen(buf), 0);
	// 		if (strstr(buf, "0\r\n\r\n"))
	// 			break;
	// 	} while (65000 - strlen(buf) > 1);

	// 	temp.clear();
	// 	temp = buf;
	// 	bzero(buff, length + 1);
	// 	temp.copy(buff, length - length_copy, 0);
	// 	ofile << buff;
	// 	temp = &temp.at(length - length_copy + 2);
	// 	content.clear();
	// }
	// content.clear();
	// ofile.close();
	return (content);
}