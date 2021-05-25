#include "Core.hpp"

// UTILS ============================================================================================

string trim_whitespace(string line)
{
	return string(line, line.find_first_not_of("\t\n\v\f\r "), line.find_last_not_of("\t\n\v\f\r ") + 1);
}

string get_key(string line)
{
	string key;
	for (string::iterator it = line.begin(); it != line.end(); it++)
	{
		if (*it == ':')
			return key;
		key.push_back(*it);
	}
	/*key.erase(key.find_last_not_of(" \n\r\t") + 1);
	char first = *key.begin();
	if ((first >= 9 && first <= 13) || first == ' ')
		key.erase(key.begin(), key.begin() + key.find_first_not_of(" \n\r\t"));*/
	return trim_whitespace(key);
}

string get_val(string line)
{
	string val;
	string::iterator it = line.begin();
	for (; it != line.end(); it++)
		if (*it == ':')
			break;
	it++;
	for (; it < line.end(); it++)
		val.push_back(*it);
	/*val.erase(val.find_last_not_of(" \n\r\t") + 1);
	char first = *val.begin();
	if ((first >= 9 && first <= 13) || first == ' ')
		val.erase(val.begin(), val.begin() + val.find_first_not_of(" \n\r\t"));*/
	//cout << "\e[1m[DEBUG 2] -> |" << val << "|\e[0m" << endl;
	if (!val.size())
	{
		cout << "\e[91mInvalid .conf format\e[0m" << endl;
		exit(0);
	}
	return trim_whitespace(val);
}

bool is_not_whitespace(string line)
{
	for (string::iterator it = line.begin(); it != line.end(); it++)
	{
		//cout << "\e[1m[DEBUG] -> |" << (int)*it << "| = |" << *it << "|\e[0m" << endl;
		if ((*it >= 9 && *it <= 13) || *it == ' ')
			continue;
		else
			return true;
	}
	return false;
}

// PARSING INDEPENDANT ================================================================================

pair<string, vector<string> > Core::parseMethod(map<string, string> args, string loc, string path, int n)
{
	vector<string> methods;

	methods.push_back("GET");
	//methods.push_back("HEAD");
	if (!args.count("allow_methods"))
		return make_pair(loc, methods);

	cout << "go" << endl;

	string list[] = {"GET", "HEAD", "POST", "PUT", "DELETE"};

	string parsed = args.find("allow_methods")->second;
	string tmp;

	for (string::iterator it = parsed.begin(); it != parsed.end(); it++)
	{
		if ((*it >= 9 && *it <= 13) || *it == ' ' || *it == ',')
		{
			if (!tmp.size())
				continue;
			for (size_t i = 0; i < 7; i++)
			{
				if (i == 6)
				{
					cout << "\e[91m[\e[1;39m" << tmp << "\e[91m] is an unexcepted method in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
					exit(1);
				}
				if (tmp == list[i])
					break;
			}
			if (!(tmp == "GET"))
				methods.push_back(tmp);
			tmp.clear();
		}
		else
			tmp.push_back(*it);
	}

	for (size_t i = 0; i < 7; i++)
	{
		if (i == 6)
		{
			cout << "\e[91m[\e[1;39m" << tmp << "\e[91m] is an unexcepted method in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
			exit(1);
		}
		if (tmp == list[i])
			break;
	}
	if (!(tmp == "GET"))
		methods.push_back(tmp);
	tmp.clear();
	return make_pair(loc, methods);
}

vector<string> Core::parseMethod(string parsed, string path, int n)
{
	vector<string> methods;

	methods.push_back("GET");

	string list[] = {"GET", "HEAD", "POST", "PUT", "DELETE"};

	string tmp;

	for (string::iterator it = parsed.begin(); it != parsed.end(); it++)
	{
		if ((*it >= 9 && *it <= 13) || *it == ' ' || *it == ',')
		{
			if (!tmp.size())
				continue;
			for (size_t i = 0; i < 7; i++)
			{
				if (i == 6)
				{
					cout << "\e[91m[\e[1;39m" << tmp << "\e[91m] is an unexcepted method in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
					exit(1);
				}
				if (tmp == list[i])
					break;
			}
			if (!(tmp == "GET"))
				methods.push_back(tmp);
			tmp.clear();
		}
		else
			tmp.push_back(*it);
	}

	for (size_t i = 0; i < 7; i++)
	{
		if (i == 6)
		{
			cout << "\e[91m[\e[1;39m" << tmp << "\e[91m] is an unexcepted method in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
			exit(1);
		}
		if (tmp == list[i])
			break;
	}
	if (!(tmp == "GET"))
		methods.push_back(tmp);
	tmp.clear();
	return methods;
}

pair<string, map<string, string> > parseLocation(int fd, int *numb, string path, string line)
{
	string loc = get_val(line);
	map<string, string> params;

	line.clear();

	bool in = false;
	bool start = false;

	int ret;
	int n = *numb;

	char c;

	while ((ret = read(fd, &c, 1)) > 0)
	{
		if (c == '#')
			while ((ret = read(fd, &c, 1)) > 0)
				if (c == '\n')
					break;
		if (c == '\t')
			continue;
		if (c != '\n')
			line.push_back(c);
		else
		{
			if (!start && line.find_first_of('{') != line.npos)
			{
				line.clear();
				start = true;
				;
				in = true;
				continue;
			}
			else if (line.find_first_of('}') != line.npos)
			{
				if (!start)
				{
					cout << "\e[91mmissing '\e[1;91m{\e[0;91m' in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
					exit(1);
				}
				in = false;
				break;
			}
			else if (!is_not_whitespace(line))
			{
				line.clear();
				continue;
			}
			else
			{
				params.insert(make_pair(get_key(line), get_val(line)));
				//DEBUG cout << "\e[97mlocation params registered: " << line << "\e[0m" << endl;
				line.clear();
			}
		}
	}
	if (in)
	{
		cout << "\e[91mmissing '\e[1;91m}\e[0;91m' in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
		exit(1);
	}
	if (!params.count("index"))
		params.insert(make_pair("index", "index.html"));
	return make_pair(loc, params);
}

map<int, string> parseErrorPages(int fd, int *numb, map<int, string> default_pages, string path)
{
	bool in = false;
	bool start = false;

	int ret;
	int n = *numb;

	char c;

	string line;

	while ((ret = read(fd, &c, 1)) >= 0)
	{
		if (c == '#')
			while ((ret = read(fd, &c, 1)) >= 0)
				if (c == '\n')
					break;
		if (c == '\t')
			continue;
		if (c != '\n')
			line.push_back(c);
		else
		{
			if (!start && line.find_first_of('{') != line.npos)
			{
				line.clear();
				start = true;
				;
				in = true;
				continue;
			}
			else if (line.find_first_of('}') != line.npos)
			{
				if (!start)
				{
					cout << "\e[91mmissing '\e[1;91m{\e[0;91m' in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
					exit(1);
				}
				in = false;
				break;
			}
			else if (!is_not_whitespace(line))
			{
				line.clear();
				continue;
			}
			else
			{
				int code = atoi(get_key(line).c_str());
				if (!code)
				{
					cout << "\e[91minvalid \e[1;91m[" << get_key(line) << "\e[0;91m] error code \e[1;91m" << path << ":" << n << "\e[0m" << endl;
					exit(1);
				}
				default_pages[code] = get_val(line);
			}
			cout << "\e[93mError pages registered: " << line << "\e[0m" << endl;
			line.clear();
		}
		if (!ret)
			break;
	}
	if (in)
	{
		cout << "\e[91mmissing \e[1;91m}\e[0;91m] in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
		exit(1);
	}
	*numb = n;
	return default_pages;
}

void Core::setWorkers(string line)
{
	int nb = 0;
	for (string::iterator it = line.begin(); it != line.end(); it++)
	{
		if (!isdigit(*it))
		{
			cout << "\e[1;91mInvalid character in workers numbers" << endl;
			exit(1);
		}
		nb = (nb * 10) + (*it - '0');
		if (nb > MAX_WORKERS)
		{
			cout << "\e[1;91mThe number of workers is limited to: " << MAX_WORKERS << endl;
			exit(1);
		}
	}
	_count_workers = nb;
}

//TODO if port exist, default_server > other

void Core::parseServer(int fd, string line, string path, int *numb)
{
	//map<string, map<string, string> Core;
	bool start = false;
	bool in = false;

	int port = DEFAULT_PORT;
	int ret;
	int n = *numb;

	char c;

	string name = "default_server";
	string root = "./default";
	string index = "index.html";

	map<int, string> error_pages;
	map<string, map<string, string> > locations;
	map<string, vector<string> > methods;
	vector<string> allowed;

	error_pages.insert(make_pair(404, "default/error.html"));
	error_pages.insert(make_pair(405, "default/error.html"));

	string possible[] = {"listen", "server_name", "root", "error_pages", "location", "index", "allow_methods"};

	line.clear();
	while ((ret = read(fd, &c, 1)) > 0)
	{
		if (c == '#')
			while ((ret = read(fd, &c, 1)) > 0)
				if (c == '\n')
					break;
		if (c == '\t')
			continue;
		else if (c != '\n')
			line.push_back(c);
		else
		{
			if (!start && line.find_first_of('{') != line.npos)
			{
				start = true;
				in = true;
				line.clear();
				continue;
			}
			else if (line.find_first_of('}') != line.npos)
			{
				if (!start)
				{
					cout << "\e[91mmissing '\e[1;91m{\e[0;91m' in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
					exit(1);
				}
				in = false;
				break;
			}
			else
			{
				if (!start)
				{
					cout << "\e[91mmissing '\e[1;91m{\e[0;91m' in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
					exit(1);
				}
				n++;
				if (!is_not_whitespace(line))
				{
					line.clear();
					continue;
				}
				for (int i = 0; i < 8 /*replace by 4 */; i++)
				{
					if (i == 7)
					{
						cout << "\e[91m[\e[1;39m" << get_key(line) << "\e[91m] is an unexcepted identifier in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
						exit(1);
					}
					if (get_key(line) == possible[i])
					{
						switch (i)
						{
						case 0:
							port = atoi(get_val(line).c_str());
							if (!port)
							{
								cout << "\e[91m[\e[1;39m" << line << "\e[91m] invalid port in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
								exit(1);
							}
							i = 7;
							break;
						case 1:
							name = get_val(line);
							i = 7;
							break;
						case 2:
							root = get_val(line);
							i = 7;
							break;
						case 3:
							error_pages = parseErrorPages(fd, &n, error_pages, path);
							i = 7;
							break;
						case 4:
							locations.insert(parseLocation(fd, &n, path, line));
							methods.insert(parseMethod(locations.find(get_val(line))->second, get_val(line), path, n));
							i = 7;
							break;
						case 5:
							index = get_val(line);
							i = 7;
							break;
						case 6:
							allowed = parseMethod(get_val(line), path, n);
							i = 7;
							break;
						default:
							break;
						}
					}
				}
			}
			//DEBUG	cout << "\e[94mdata server registered: " << line << "\e[0m" <<endl;
			line.clear();
		}
	}
	if (in)
	{
		cout << "\e[91mmissing '\e[1;91m}\e[0;91m' in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
		exit(1);
	}
	if (!allowed.size())
	{
		allowed.push_back("GET");
		//allowed.push_back("HEAD");
	}
	*numb = n;
	_t_preServ preServ = {_pre_Serv.size(), port, name, root, error_pages, locations, index, methods, allowed};

	_pre_Serv.push_back(preServ);
}

// PARSIN =================================================================================================

Core::Core(string path)
{
	int fd;
	fd = open(path.c_str(), O_RDONLY | O_DIRECTORY | O_NONBLOCK);
	if (fd != -1)
	{
		cout << "\e[91mWebServ: " << path << ": is a directory." << endl;
		exit(1);
	}
	close(fd);
	fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
	if (fd < 3)
	{
		cout << "\e[91mWebServ: " << path << ": canno't open file." << endl;
		exit(1);
	}
	_count_workers = 1;
	//mettre 1 a 1 dans line jusqu'a \n
	// traiter la ligne
	// reset la ligne,
	string possible[] = {"workers", "events", "server"};
	char c;
	string line;
	int ret;
	int n = 0;
	while ((ret = read(fd, &c, 1)) > 0)
	{
		if (c == '#')
			while ((ret = read(fd, &c, 1)) > 0)
				if (c == '\n')
					break;
		if (c != '\n')
			line.push_back(c);
		else
		{

			n++;
			if (!is_not_whitespace(line))
				continue;
			for (int i = 0; i < 3 /*replace by 4 */; i++)
			{
				if (i == 3)
				{
					cout << "\e[91m[\e[1;39m" << get_key(line) << "\e[91m] is an unexcepted identifier in \e[1;91m" << path << ":" << n << "\e[0m" << endl;
					exit(1);
				}
				if (get_key(line) == possible[i])
				{
					switch (i)
					{
					case 0:
						setWorkers(get_val(line));
						i = 4;
						break;
					case 1:
						i = 4;
						break;
					case 2:
						parseServer(fd, line, path, &n);
						i = 4;
						break;
					default:
						break;
					}
				}
				/*else
					cout << "|" << line << "| =/= |" << possible[i] << "|"<<endl;*/
			}
			//DEBUG cout << "\e[92mOK for :" << line << "\e[0m" << endl;
			line.clear();
		}
	}
	pthread_mutex_t	*logger = new pthread_mutex_t;
	pthread_mutex_init(logger, NULL);
	close(fd);
	for (size_t j = 0; j < _pre_Serv.size(); j++)
		_servers.push_back(new Server(_pre_Serv[j], logger));
	for (int i = 0; i < _count_workers; i++)
	{
		Worker *worker = new Worker(i);
		_workers.insert(make_pair(i, worker));
	}
}

Core::~Core()
{
	//close(_fd);
	// for (int i = 0; i < getCountWorkers(); i++)
	// 	delete this->getWorkers().find(i)->second->getThread();
}

// Hearth of webserv
void Core::run(map<int, Worker *> &workers, int count)
{
	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = 20;
	// _count_requests = 0;
	// (void)workers[count];

	while (1)
	{
		size_t i = 0;
		for (i = 0; i < getServers().size(); i++)
		{
			// Prepare variables for current server
			socklen_t clilen = sizeof(getServers().at(i)->getCliAddr());
			sockaddr_in cli_addr = getServers().at(i)->getCliAddr();
			fd_set write_fds = getServers().at(i)->getWriteFD();
			fd_set read_fds = getServers().at(i)->getReadFD();
			fd_set write_fds2 = getServers().at(i)->getWriteFD();
			fd_set read_fds2 = getServers().at(i)->getReadFD();
			for (int l = 0; l < count; l++)
				workers.find(l)->second->setSocket(0);
			if (select(getServers().at(i)->getSocket() + 1, &read_fds2, &write_fds2, NULL, &tv) > 0)
			{
				// Search for an available worker
				int j;
				while (1)
				{
					j = 0;

					while (workers.find(j)->second->getStatus() && j < count - 1)
						j++;
					if (workers.find(j)->second->getStatus())
						break;
				}
				workers.find(j)->second->setServer(getServers().at(i));
				workers.find(j)->second->setSocket(22);
				if (!FD_ISSET(getServers().at(i)->getSocket(), &write_fds) || !FD_ISSET(getServers().at(i)->getSocket(), &read_fds))
					// error("ERROR non-set socket");
					;
				int newsockfd = accept(getServers().at(i)->getSocket(), (struct sockaddr *)&cli_addr, &clilen);
				// if (newsockfd < 0)
				// 	error("ERROR on accept");
					cout << "jen ressors jamais1 " << endl;
				getServers().at(i)->handle_request(newsockfd);
				cout << "jen ressors jamais2 " <<endl;
				close(newsockfd);
			}
			else
				getServers().at(i)->log("\e[1;96m[IDLING]\e[0m");
		}
	}
}

Core &Core::operator=(const Core & other)
{
	//if (other == this)
	//	return *this;
	_count_workers = other._count_workers;
	_servers = other._servers;
	_workers = other._workers;
	return *this;
}

// Check if the client exist with a socket as entry parameter
bool Core::exists(int socket)
{
	list<Client>::iterator begin = _clients.begin();
	while (begin != _clients.end())
		if (begin->getSocket() == socket)
			return (true);
	return (false);
}

int Core::getCountWorkers() { return _count_workers; }
map<int, Worker *> &Core::getWorkers() { return _workers; }
vector<Server *> Core::getServers() { return _servers; }