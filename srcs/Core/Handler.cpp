#include "Utils.hpp"

// POST function
int post_request(map<string, string> request)
{
	struct stat sb;
	string content = request.find("Content")->second;
	string path;
	const char *content_char = content.c_str();
	const char *to_print = content_char;
	if (!request.count("Content") || !request.find("Content")->second.size())
	{
		dprintf(1, "\e[91mPOST : Content vide!\e[0m\n");
		return (ERROR);
	}
	// Get the path + check the "/" path case
	if (request.find("Location")->second == "/")
		;
	else if (stat(request.find("Location")->second.c_str(), &sb) != -1)
	{
		int nb_prints, fd = open(request.find("Location")->second.c_str(), O_TRUNC | O_WRONLY, 0777);
		istringstream iss(request.find("Content-Length")->second);
		size_t remaining_characters, count = 0;
		iss >> remaining_characters;
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
	}
	else
	{
		dprintf(1, "\e[35mfichier existe pas mon frere\e[0m\n");
		return (ERROR);
	}
	return (SUCCESS);
}

// Dispatcher depending on type in request (GET, POST, HEAD)
int dispatcher_type_requests(map<string, string> request)
{
	string possibility_requests[5] = {"GET", "POST", "PUT", "HEAD", "DELETE"};
	string *res;
	// POST check si j'ai les methodes allowed
	if ((res = find(possibility_requests, possibility_requests + 5, request.find("Request-Type")->second)))
	{
		if (*res == "GET")
			dprintf(1, "GET\n");
		else if (*res == "POST")
			post_request(request);
		else if (*res == "PUT")
			dprintf(1, "PUT\n");
		else if (*res == "HEAD")
			dprintf(1, "HEAD");
		else if (*res == "DELETE")
			dprintf(1, "DELETE\n");
		else
			return (ERROR);
	}
	return (SUCCESS);
}