#include "Server.hpp"

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

	FD_ZERO(&_write_fds);
	FD_ZERO(&_read_fds);

	FD_SET(_socket, &_write_fds);
	FD_SET(_socket, &_read_fds);
}

Server::Server(_t_preServ pre)
{
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

	FD_ZERO(&_write_fds);
	FD_ZERO(&_read_fds);

	FD_SET(_socket, &_write_fds);
	FD_SET(_socket, &_read_fds);

	_id = pre.id;
	_name = pre.name;
	_root = pre.root;
	_error_pages = pre.err;
	_locations = pre.loc;
	_index = pre.index;
	_locations_methods = pre.methods;
	_allowed = pre.allowed;
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
			handle_request(newsockfd);
			close(newsockfd);
		}
		else
			log("\e[1;96m[IDLING]\e[0m");
	}
}

#include "Headers.hpp"

void Server::handle_request(int sock)
{
	
	string type[] = {"GET", "POST", "HEAD"};
	string(Server::*command[])(map<string, string>) = {&Server::GET, &Server::POST, &Server::HEAD};
	char buffer[4096];
	bzero(buffer, 4096);
	int n = read(sock, buffer, 4096);

	if (n < 0)
		error("Can't read the request");
	
	Headers request;

	request += string(buffer);

	string response;

	map<string, string>p_request = request.last();
	
	string req_type = p_request.find("Request-Type")->second;

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
				response = (this->*command[i])(p_request);
			else
				response = SEND_ERROR(STATUS_METHOD_NOT_ALLOWED, "Method Not Allowed");
			break;
		}
	}
	n = write(sock, response.c_str(), strlen(response.c_str()));
	if (n < 0)
		error("Can't send the response");

	
	// TODO: CHECK HEADER

	// HANDLING REQUEST


	
	
	
	
	
	
	
	
	//=======================================
	
	
	
	
	/*
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
		error("ERROR writing to socket_nbet");*/
}

void Server::error(const char *s)
{
	perror(s);
	exit(1);
}


//PAS FINIS
string Server::POST(map<string, string> header)
{

	log("\e[1;93m[POST -> " + header.find("Location")->second + "]\e[0m");

	Headers tmp;
	string resp;
	t_file file = getFile(header.find("Location")->second);

	if (header.count("Content") && !header.find("Content")->second.size())
		resp = SEND_ERROR(STATUS_NO_CONTENT, "No Content");
	return resp;
}

// PAS FAIS
string Server::HEAD(map<string, string> header)
{
	t_file file = getFile(header.find("Location")->second);

	return file.content;
}

//A AMELIORER
string Server::GET(map<string, string> header)
{

	log("\e[1;93m[GET -> " + header.find("Location")->second + "]\e[0m");

	t_file file = getFile(header.find("Location")->second);
	Headers tmp;
	string resp;

cout << header.count("coffee") << endl;
	if (file.content == "NOT FOUND" && file.size == 0)
		resp = SEND_ERROR(STATUS_NOT_FOUND, "Not Found");
	else if (header.count("coffee"))
		resp = SEND_ERROR(STATUS_TEAPOT, "I'm a teapot");
	else
	{
		resp = tmp.return_response_header(200, tmp, file.size);
		resp += file.content;
	}

	return resp;
}


// SOUCIS: si on  location /foo/ et location /foo/bar/ dans le .conf et aue l'url est /foo/bar/, il va chopper /foo/
// il faut donc le comparer avec tout et prendre celui avec le + de char correspondant
t_file Server::getFile(string loc)
{

	map<string, map<string, string> >::iterator it = _locations.begin();

	size_t pos;
	string parsed = "";

// FIND IF THE START OF THE URL IS PRESENT IN CONFIG

	for(size_t i = 0; i < _locations.size(); i++)
	{
			//rfind = find prefix (si dans l'url /foo/bar/ je trouve la location /foo/ alors pos != npos)
			pos = loc.rfind(it->first, 0);
			if (pos != loc.npos)
			{
				// SKIP BECAUSE IT MATCH IF IN CONF WE HAVE / && /foo AND GO TO / EVEN IF THE URL CONTAINS /foo
				if (_locations.size() > 1 && it->first == "/")
				{
					it++;
					continue;
				}
				//replace la location par la root de la location
				if (it->second.count("root"))
					loc.replace(pos, it->first.size() , it->second.find("root")->second);
				parsed = loc;
				break;
			}
		it++;
	}
	// REGET THE / LOCATION IF NO OTHER HAS BEEN FOUND
	if (parsed == "" && _locations.count("/"))
	{
		if (_locations.find("/")->second.count("root"))
			loc.replace(0, 1, _locations.find("/")->second.find("root")->second);
		parsed = loc;
	}

// IF NOT, GET THE DEFAULT ROOT
	if (parsed == "")
		parsed = _root + loc;

	//cout << "Final Location: " << parsed << endl;

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
	while(read(fd, &c, 1))
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

	gettimeofday(&tv, NULL);
	t = tv.tv_sec;

	info = localtime(&t);
	strftime(buffer, sizeof buffer, "[%c]", info);

	cout << "\e[1;" << 92 + _id << "m[SERVER " << _id << "]\e[0m" << s << " \e[1;90m" << buffer << "\e[0m"<< endl;
}

bool Server::check_methods(map<string, string> req)
{
	string methods = req.find("Request-Type")->second;
	string loc = req.find("Location")->second;
	map<string, map<string, string> >::iterator it = _locations.begin();
	size_t pos;

	for(size_t i = 0; i < _locations.size(); i++)
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
				for(size_t i = 0; i < allowed.size(); i++)
					if(methods == allowed[i])
						return true;
				return false;
			}
		it++;
	}
	for(size_t i = 0; i < _allowed.size(); i++)
		if(methods == _allowed[i])
			return true;
	return false;
}