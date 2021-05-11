#include "Headers.hpp"

Headers::Headers()
{
}

Headers::~Headers()
{
}

string::iterator get_ite(string tmp, size_t pos)
{
	return tmp.begin() + pos;
}
//Request-Type nom de la key pour le POST, GET etc
//Location == /foo/bar
//HTTP-Ver version HTTP
void Headers::operator+=(string send)
{
	istringstream to_parse(send);
	map<string, string> head;
	string line;

	while(getline(to_parse,line))
	{
		if(line == "\r")
		{

			getline(to_parse,line);

			head.insert(make_pair("Content", line));
			continue;
		}
		line[line.size() - 1] = '\0';
		if(head.size() == 0)
		{
			string type(line, 0, line.find('/') - 1);
			head.insert(make_pair("Request-Type", type));
			string loc(line, type.size() + 1, line.find("HTTP") - 6);
			head.insert(make_pair("Location", "default" + loc));
			string ver(line, line.find_last_of('/') + 1);
			head.insert(make_pair("HTTP-Ver", ver));
		}
		else
		{
			string index(line, 0, line.find_first_of(':'));
			string content(line, line.find_first_of(':') + 2, line.size() - 2);
			head.insert(make_pair(index, content));
		}
	}
	_headers.insert(make_pair(_headers.size(), head));
}

int Headers::check(map<string, string> actual)
{
	dprintf(1, "%s\n", actual.find("HTTP-Ver")->second.c_str());
	string ver = actual.find("HTTP-Ver")->second;
	if (strncmp(actual.find("HTTP-Ver")->second.c_str(), "1.1", 3))
		return 1; // 505 bad version
	if (actual.find("Location")->second.size() > 100000000)
		return 2; // 414 too long uri
	if (!actual.count("Host"))
		return 3; // 400 no host
	return 0;
}