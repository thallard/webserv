#include "Config.hpp"

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

pair<string, map<string, string> > parseLocation(int fd, int *numb, char *path, string line)
{
	string loc = get_val(line);
	map<string, string> params;

	line.clear();

	bool in = false;
	bool start = false;

	int ret;
	int n = *numb;

	char c;

	(void)fd;
	(void)numb;
	(void)path;
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
				start = true;;
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
				cout << "\e[97mlocation params registered: " << line << "\e[0m" << endl;
				line.clear();
			}
		}
	}
	if (in)
	{
		cout << "\e[91mmissing '\e[1;91m}\e[0;91m' in \e[1;91m" << path<< ":" << n << "\e[0m" << endl;
		exit(1);
	}
	return make_pair(loc, params);
}

map<int, string> parseErrorPages(int fd, int *numb, map<int, string>default_pages, char *path)
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
				start = true;;
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
					cout << "\e[91minvalid \e[1;91m["<< get_key(line) <<"\e[0;91m] error code \e[1;91m" << path << ":" << n << "\e[0m"<< endl;
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
		cout << "\e[91mmissing \e[1;91m}\e[0;91m] in \e[1;91m" << path << ":" << n << "\e[0m"<< endl;
		exit(1);
	}
	*numb = n;
	return default_pages;
}

void Config::setWorkers(string line)
{
	int nb = 0;
	for (string::iterator it = line.begin(); it != line.end(); it++)
	{
		if (!isdigit(*it))
		{
			cout << "\e[1;91mInvalid charactere in workers numbers" << endl;
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

Server Config::parseServer(int fd, string line, char *path, int *numb)
{
	//map<string, map<string, string> config;
	bool start = false;
	bool in = false;

	int port = DEFAULT_PORT;
	int ret;
	int n = *numb;

	char c;

	string name = "default_server";
	string root = "default/";

	map<int, string> error_pages;
	map<string, map<string, string> > locations;
	
	error_pages.insert(make_pair(404, "default/error.html"));
	error_pages.insert(make_pair(405, "default/error.html"));

	string possible[] = {"listen", "server_name", "root", "error_pages", "location"};

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
					cout << "\e[91mmissing '\e[1;91m{\e[0;91m' in \e[1;91m" << path << ":" << n << "\e[0m"<< endl;
					exit(1);
				}
				in = false;
				break;
			}
			else
			{
				if (!start)
				{
					cout << "\e[91mmissing '\e[1;91m{\e[0;91m' in \e[1;91m" << path<< ":" << n << "\e[0m" << endl;
					exit(1);
				}
				n++;
			if (!is_not_whitespace(line))
			{
				line.clear();
				continue;
			}
			for (int i = 0; i < 6 /*replace by 4 */; i++)
			{
				if (i == 5)
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
							cout << "\e[91m[\e[1;39m" << line << "\e[91m] invalid port in \e[1;91m" << path << ":" << n << "\e[0m"<< endl;
							exit(1);
						}
						i = 6;
						break;
					case 1:
						name = get_val(line);
						i = 6;
						break;
					case 2:
						root = get_val(line);
						i = 6;
						break;
					case 3:
						error_pages = parseErrorPages(fd,  &n, error_pages, path);
						i = 6;
						break;
					case 4:
						locations.insert(parseLocation(fd, &n, path, line));
						i = 6;
						break;
					default:
						break;
					}
				}
			}
		}
		cout << "\e[94mdata server registered: " << line << "\e[0m" <<endl;
		line.clear();
	}

	}
	if (in)
	{
		cout << "\e[91mmissing '\e[1;91m}\e[0;91m' in \e[1;91m" << path<< ":" << n << "\e[0m" << endl;
		exit(1);
	}
	*numb = n;
	Server server(port);
	server.setId(_servers.size());
	server.setName(name);
	server.setRoot(root);
	server.setErrorPages(error_pages);
	server.setLocations(locations);
	return server;
}



// PARSIN =================================================================================================

Config::Config(char *path)
{
	int fd;
	fd = open(path, O_RDONLY | O_DIRECTORY | O_NONBLOCK);
	if (fd != -1)
	{
		cout << "\e[91mWebServ: " << path << ": is a directory." << endl;
		exit(1);
	}
	close(fd);
	fd = open(path, O_RDONLY | O_NONBLOCK);
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
						_servers.push_back(parseServer(fd, line, path, &n));
						i = 4;
						break;
					default:
						break;
					}
				}
				/*else
					cout << "|" << line << "| =/= |" << possible[i] << "|"<<endl;*/
			}
			cout << "\e[92mOK for :" << line << "\e[0m" << endl;
			line.clear();
		}
	}
	close(fd);
	for (int i = 0; i < _count_workers; i++)
	{
		Worker *worker = new Worker(i);
		_workers.insert(make_pair(i, worker));
	}
}

Config::~Config() 
{
	// for (int i = 0; i < getCountWorkers(); i++)
	// 	delete this->getWorkers().find(i)->second->getThread();
}

int Config::getCountWorkers() { return _count_workers; }
map<int, Worker *> Config::getWorkers() { return _workers;}
vector<Server> Config::getServers() { return _servers; }
// map<int, Worker