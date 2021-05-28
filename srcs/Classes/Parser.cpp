#include "Parser.hpp"

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
		if ((*it >= 9 && *it <= 13) || *it == ' ')
			continue;
		else
			return true;
	}
	return false;
}

// LOCATION TREE CREATOR ========================================================================================

void Parser::finishLoc(t_loc *parent, t_loc *child)
{
//cout << "Im \e[92m" << child->path << "\e[0m and my parent is \e[93m" << parent->path << "\e[0m" << endl;
	if (!child->options.methods.size())
		child->options.methods = parent->options.methods;

	for (map<string, string>::iterator it = parent->options.params.begin(); it != parent->options.params.end(); it++)
	{
		if ((*it).first == "root")
			continue;
		child->options.params.insert(*it);
	}
	string parent_root = parent->options.params.find("root")->second;
	string path;

	if (parent_root[parent_root.size() - 1] == '/')
		parent_root = parent_root.substr(0, parent_root.size() - 1);

	if (!child->options.params.count("root"))
		path = child->path;
	else
		path = child->options.params.find("root")->second;


	if (path[path.size() - 1] == '/')
			path = path.substr(0, path.size() - 1);

	if (!child->options.params.count("root"))
		child->options.params.insert(make_pair("root", parent_root + "/" + path + "/"));
	else
		child->options.params.find("root")->second = parent_root + "/" + path + "/";


	for (list<t_loc *>::iterator it = child->childs.begin(); it != child->childs.end(); it++)
		finishLoc(child, (*it));
}



void Parser::addToLoc(map<string, string> loc, vector<string> meth, string path, t_loc *root, string dir, int n)
{
	string save = path;
	t_loc *child;
	t_loc *parent;

	child = root;
	size_t pos = 0;

	string token;
	if (path.size() > 1 && path[path.size() - 1] == '/')
			path = path.substr(0, path.size() - 1);
	while ((pos = path.find("/")) != string::npos)
	{
		if (path[0] == '/')
		{
			path.erase(0, 1);
			continue;
		}

		token = path.substr(0, pos);
		if(token.size())
		{
			//cout << token << endl;
			parent = child;
				for (list<t_loc *>::iterator it = child->childs.begin(); it != child->childs.end(); it++)
				{
					if ((*it)->path == token)
					{
						//cout << "\e[92mfound: " << token << " in -> " << parent->path << "\e[0m" <<endl;
						child = *it;
						break;
					}
				}
			if (child == parent)
			{
				//cout << "\e[91mnot found: " << token << " in -> " << parent->path << "\e[0m" << endl;
				t_loc *new_child = new t_loc();
				new_child->path = token;
				parent->childs.push_front(new_child);
				child = *parent->childs.begin();
				child->parent = parent;
			}
		}
		path.erase(0, pos + 1);
	}
	//cout << path << endl;
	if (path.size())
	{
	parent = child;
		if (child->childs.size())
		{
			for (list<t_loc *>::iterator it = child->childs.begin(); it != child->childs.end(); it++)
			{
				if ((*it)->path == path)
				{
					if (child->options.methods.size())
					{
						cout << "\e[91m[\e[1;39m" << save << "\e[91m] duplicate location \e[1;91m" << dir << ":" << n << "\e[0m" << endl;
						exit(1);
					}
					//cout << "\e[92mfound: " << token << " in -> " << parent->path << " but was not set\e[0m" <<endl;
					child->options.methods = meth;
					child->options.params = loc;
					child = *it;
					break;
				}
			}
		}
		if (child == parent)
		{
			//cout << "\e[91mnot found: " << path << " in -> " << parent->path << "\e[0m" << endl;
			t_loc *new_child = new t_loc();
			new_child->path = path;
			parent->childs.push_front(new_child);
			child = *parent->childs.begin();
			child->options.methods = meth;
			child->options.params = loc;
			child->parent = parent;
		}
	}
	//cout << "=============" << endl;
}

// PARSING INDEPENDANT ================================================================================


// Get the allowed method IN location: x

vector<string>Parser::parseMethod(map<string, string> args, string loc, string path, int n)
{
	vector<string> methods;
	(void)loc;

	methods.push_back("GET");
	//methods.push_back("HEAD");
	if (!args.count("allow_methods"))
		return methods;

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
	return methods;
}

// Get the allowed method OUT location: x

vector<string> Parser::parseMethod(string parsed, string path, int n)
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

// Get params of a specific location

map<string, string> Parser::parseLocation(int fd, int *numb, string path, string line)
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
	return params;
}

// Get error pages

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


// Tree of locations
void printLoc(t_loc *root, int level)
{
	t_loc *tmp = root;
	int i = 0;
	for(list<t_loc *>::iterator it = tmp->childs.begin(); it != tmp->childs.end(); it++)
	{
		i = 0;
		while(i++ < level)
			cout << "  ";
		cout << "\e[" << to_string(92 + level) << "mL-> " << (*it)->path  << "\e[0m (root: " << (*it)->options.params.find("root")->second << ")" << endl;
		printLoc(*it, level + 1);
	}
}


void Parser::setWorkers(string line)
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

void Parser::parseServer(int fd, string line, string path, int *numb)
{
	//map<string, map<string, string> Parser;
	bool start = false;
	bool in = false;

	int port = DEFAULT_PORT;
	int ret;
	int n = *numb;

	char c;

	t_loc *loc = new t_loc();
	loc->path = "/";
	loc->parent = NULL;

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
							addToLoc(parseLocation(fd, &n, path, line), parseMethod(locations.find(get_val(line))->second, get_val(line), path, n), get_val(line), loc, path, n);
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
	if (!loc->options.methods.size())
		loc->options.methods = allowed;
	if (!loc->options.params.count("root"))
		loc->options.params.insert(make_pair("root", root));
	for (list<t_loc *>::iterator it = loc->childs.begin(); it != loc->childs.end(); it++)
		finishLoc(loc, (*it));
	
	if (!loc->options.params.count("index"))
		loc->options.params.insert(make_pair("index", index));

	cout << "Server: \e[97;1m" << name << endl;
	cout << "\e[0m\e[91mroot /\e[0m" << endl;
		printLoc(loc, 0);
	cout << endl << endl;

	*numb = n;
	_t_preServ preServ = {_pre_Serv.size(), port, name, error_pages,loc};

	_pre_Serv.push_back(preServ);
}

// PARSIN =================================================================================================

Parser::Parser(string path)
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

	close(fd);
}

Parser::~Parser()
{

}


Parser &Parser::operator=(const Parser &other)
{
	//if (other == this)
	//	return *this;
	_count_workers = other._count_workers;
	_pre_Serv = other._pre_Serv;
	return *this;
}

int Parser::getCountWorkers() { return _count_workers; }
vector<_t_preServ> Parser::getPreServ() { return _pre_Serv; }