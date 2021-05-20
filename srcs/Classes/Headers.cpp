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

	while (getline(to_parse, line))
	{
		if (line == "\r")
		{

			getline(to_parse, line);

			head.insert(make_pair("Content", line));
			continue;
		}
		line[line.size() - 1] = '\0';
		if (head.size() == 0)
		{
			string type(line, 0, line.find('/') - 1);
			head.insert(make_pair("Request-Type", type));
			string loc(line, type.size() + 1, line.find("HTTP") - 5);
			head.insert(make_pair("Location", loc));
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
	string ver = actual.find("HTTP-Ver")->second;
	if (strcmp(actual.find("HTTP-Ver")->second.c_str(), "1.1"))
		return 505; // 505 bad version
	if (actual.find("Location")->second.size() > 100000000)
		return 414; // 414 too long uri
	if (!actual.count("Host"))
		return 400; // 400 no host
	if (!strcmp(actual.find("Request-Type")->second.c_str(), "POST") && !actual.find("Content")->second.size())
		return 204;
	return 200;
}

string Headers::return_response_header(int status, Headers header, size_t size_content)
{
	(void)status;
	(void)header;
	string response;

	// Parse status code
	switch (status)
	{
	case STATUS_OK:
		response = "HTTP/1.1 200 OK\n\r";
		break;
	case STATUS_NO_CONTENT:
		response = "HTTP/1.1 204 No Content\n\r";
		break;
	case STATUS_NOT_FOUND:
		response = "HTTP/1.1 404 Not Found\n\r";
		break;
	case STATUS_METHOD_NOT_ALLOWED:
		response = "HTTP/1.1 405 Method Not Allowed\n\r";
		break;
	case STATUS_TEAPOT:
		response = "HTTP/1.1 418 I'm a teapot\n\r";
		break;
	default:
		break;
	}

	// Parse date
	struct timeval tv;
	time_t t;
	struct tm *info;
	char buffer[64];

	gettimeofday(&tv, NULL);
	t = tv.tv_sec;

	info = localtime(&t);
	strftime(buffer, sizeof buffer, " ", info);
	response += "Date:";
	response += buffer;
	strftime(buffer, sizeof buffer, "%p %I:%M GMT +2.\n\r", info);
	response += asctime(info);
	response += "Server: webserv\n\r";
	// If method is GET/HEAD, its gonna fill this line
	if (size_content > 0)
		response += "Content-Length: " + std::to_string(size_content) + "\n\r";
	response += "Content-Language: fr-FR\n\r\n\r";
	return (response);
}