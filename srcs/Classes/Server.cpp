#include "Server.hpp"
#include "Utils.hpp"
#include "Client.hpp"

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

	FD_ZERO(&_clients_fd);
	FD_SET(_socket, &_clients_fd);
}

Server::Server(_t_preServ pre, pthread_mutex_t *logger)
{
	// Init thread
	thread = new pthread_t;
	_logger = logger;
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
	_serv_addr.sin_port = htons(pre.port);
	if (bind(_socket, (struct sockaddr *)&_serv_addr,
			 sizeof(_serv_addr)) < 0)
		perror("ERROR on binding");

	listen(_socket, 4096);

	FD_ZERO(&_clients_fd);

	FD_SET(_socket, &_clients_fd);

	_id = pre.id;
	_name = pre.name;
	_root = pre.root;
	_error_pages = pre.err;
	_locations = pre.loc;
	_index = pre.index;
	_locations_methods = pre.methods;
	_allowed = pre.allowed;
	root = pre._root;
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

	_locations = ref._locations;
	_error_pages = ref._error_pages;

	_index = ref._index;
	_locations_methods = ref._locations_methods;
	_allowed = ref._allowed;
	root = ref.root;

	return *this;
}

// Hearth of the request for the server
void Server::run(map<int, Worker *> &workers)
{
	struct timeval tv = {1, 0};
	while (1)
	{
		fd_set temp_fd = _clients_fd;
		if (select(_socket + 1, &temp_fd, NULL, NULL, &tv) > 0)
		{
			int index = findAvailableWorker(workers);
			workers.find(index)->second->setServer(this);
			while (!workers.find(index)->second->getStatus())
				;
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

void Server::handle_request(Client client)
{
	cout << "fdp de ta mere\n";
	string type[] = {"GET", "POST", "HEAD", "PUT"};
	string (Server::*command[])(map<string, string>, int) = {&Server::GET, &Server::POST, &Server::HEAD, &Server::PUT};
	string buffer = client.getContent();
	int n;
	// Read until header until is finished
	// bzero(buffer, 4096);
	// int n = read(client->getSocket(), buffer, 4096);
	cout << "Request :" << endl
		 << buffer << endl
		 << "=====" << endl;
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
			response = "404"; // change with bad formated header
			break;
		}
		if (!strcmp(req_type.c_str(), type[i].c_str()))
		{

			if (check_methods(p_request))
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
	n = write(client.getSocket(), response.c_str(), response.size());
	//n = write(sock, response.c_str(), strlen(response.c_str()));
	// cout << "allo4\n";
	if (n < 0)
		error("Can't send the response");
	close(client.getSocket());
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

	// t_file file = getFile(header.find("Location")->second);
	Headers tmp;
	string resp;

	// if (file.content == "NOT FOUND" && file.size == 0)
	resp = SEND_ERROR(STATUS_NOT_FOUND, "Not Found");
	if (header.count("coffee"))
		resp = SEND_ERROR(STATUS_TEAPOT, "I'm a teapot");
	else
	{
		resp = tmp.return_response_header(200, tmp, 0);
		resp += "oui";
	}
	cout << "je veux sortir sac a merde\n";
	return resp;
}

pair<string, map<string, string> > Server::getConfLoc(string loc)
{
	map<string, map<string, string> >::iterator save = _locations.find("/");
	size_t max_found = loc.npos;
	size_t pos;
	map<string, map<string, string> >::iterator it = _locations.begin();

	for (; it != _locations.end(); it++)
	{
		pos = loc.rfind(it->first);
		if (max_found == loc.npos)
		{
			max_found = pos;
			save = it;
		}
		else if (max_found != loc.npos && pos != loc.npos && pos > max_found)
		{
			max_found = pos;
			save = it;
		}
	}
	return *save;
}

// SOUCIS: si on  location /foo/ et location /foo/bar/ dans le .conf et aue l'url est /foo/bar/, il va chopper /foo/
// il faut donc le comparer avec tout et prendre celui avec le + de char correspondant
t_file Server::getFile(string loc)
{

	/*pair<string ,map<string, string> > match_p = getConfLoc(loc);
	map<string, string> matched = match_p.second;
	string parsed = _root;
	if (*(--(parsed.end())) != '/')
		parsed += "/";
	if (matched.count("root"))
		parsed += matched.find("root")->second;
	loc.replace(match_p.first.size());*/

	map<string, map<string, string> >::iterator it = _locations.begin();

	size_t pos = loc.npos;
	string parsed = "";

	// FIND IF THE START OF THE URL IS PRESENT IN Core

	for (size_t i = 0; i < _locations.size(); i++)
	{
		//rfind = find prefix (si dans l'url /foo/bar/ je trouve la location /foo/ alors pos != npos)
		pos = loc.rfind(it->first, 0);
		if (pos != loc.npos)
		{
			// SKIP BECAUSE IT MATCH IF IN CONF WE HAVE location: / && location: /foo IT GOES TO / EVEN IF THE URL CONTAINS /foo
			if (_locations.size() > 1 && it->first == "/")
			{
				it++;
				continue;
			}
			// replace la location par la root de la location
			if (it->second.count("root"))
				loc.replace(pos, it->first.size(), it->second.find("root")->second);
			parsed = loc;
			break;
		}
		it++;
	}
	// RE-GET THE / LOCATION IF NO OTHER HAS BEEN FOUND
	if (parsed == "" && _locations.count("/"))
	{
		if (_locations.find("/")->second.count("root"))
			loc.replace(0, 1, _locations.find("/")->second.find("root")->second);
		parsed = loc;
	}

	// IF NOT, GET THE DEFAULT ROOT
	if (parsed == "")
		parsed = _root + loc;

	//	cout << "Final Location: " << parsed << endl;

	int fd;
	fd = open(parsed.c_str(), O_RDONLY | O_DIRECTORY | O_NONBLOCK);
	if (fd != -1)
	{
		if (pos != loc.npos)
			parsed += it->second.find("index")->second;
		else
			parsed += _index;
	}
	close(fd);

	//cout << "Final Location after directory: " << parsed << endl;

	fd = open(parsed.c_str(), O_RDONLY | O_NONBLOCK);
	if (fd < 3)
	{
		close(fd);
		t_file not_found = {"NOT FOUND", 0};
		return not_found;
	}
	string page;
	int ret;
	char c;
	while ((ret = read(fd, &c, 1)) > 0)
		page.push_back(c);
	close(fd);
	t_file file = {page, page.size()};
	return file;
}

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
	cout << "\e[1;" << to_string(92 + _id) << "m[SERVER " << _id << "]\e[0m" << s << " \e[1;90m" << buffer << "\e[0m" << endl;

	pthread_mutex_unlock(_logger);
}

bool Server::check_methods(map<string, string> req)
{
	string methods = req.find("Request-Type")->second;
	string loc = req.find("Location")->second;
	map<string, map<string, string> >::iterator it = _locations.begin();
	size_t pos;

	for (size_t i = 0; i < _locations.size(); i++)
	{
		pos = loc.rfind(it->first, 0);
		if (pos != loc.npos)
		{
			// SKIP BECAUSE IT MATCH IF IN CONF WE HAVE / && /foo AND GO TO / EVEN IF THE URL CONTAINS /foo
			if (_locations.size() > 1 && it->first == "/")
			{
				it++;
				continue;
			}
			vector<string> allowed = _locations_methods.find(it->first)->second;
			for (size_t i = 0; i < allowed.size(); i++)
				if (methods == allowed[i])
					return true;
			return false;
		}
		it++;
	}
	for (size_t i = 0; i < _allowed.size(); i++)
		if (methods == _allowed[i])
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