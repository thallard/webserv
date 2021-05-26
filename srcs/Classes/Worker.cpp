#include "Worker.hpp"

Worker::Worker()
{
	is_available = true;
	this->server = NULL;
	this->socket = 0;
	thread = new pthread_t;
}

Worker::Worker(int id)
{
	this->id = id;
	this->socket = 0;
	this->is_available = true;
	this->thread = new pthread_t;
	this->server = NULL;
}

Worker::~Worker()
{
	delete thread;
}

bool Worker::getStatus()
{
	return (is_available);
}

int Worker::getId()
{
	return (id);
}

int Worker::getSocket()
{
	return (socket);
}

pthread_t *Worker::getThread()
{
	return (thread);
}

Server *Worker::getServer()
{
	return server;
}

void Worker::setStatus(bool status)
{
	is_available = status;
}

void Worker::setSocket(int sock)
{
	socket = sock;
}

void Worker::setServer(Server *server)
{
	this->server = server;
}

void Worker::handle_request(int sock)
{
	// string type[] = {"GET", "POST", "HEAD", "PUT"};
	// string (Worker::*command[])(map<string, string>, int) = {&Worker::GET, &Worker::POST, &Worker::HEAD, &Worker::PUT};
	// char buffer[4096];

	// // Read until header until is finished
	// bzero(buffer, 4096);
	// int n = read(sock, buffer, 4096);
	// // cout << buffer << endl;

	// Headers request;
	// request += string(buffer);
	// string response;
	// map<string, string> p_request = request.last();

	// string req_type = p_request.find("Request-Type")->second;

	// for (size_t i = 0; i <= type->size(); i++)
	// {
	// 	if (i == type->size())
	// 	{
	// 		response = "404"; // change with bad formated header
	// 		break;
	// 	}
	// 	if (!strcmp(req_type.c_str(), type[i].c_str()))
	// 	{
	// 		if (this->getServer()->check_methods(p_request))
	// 			response = (this->*command[i])(p_request, sock);
	// 		else
	// 		{
	// 			this->getServer()->log("\e[1;93mMethod Not Allowed!\e[0m");
	// 			response = SEND_ERROR(STATUS_METHOD_NOT_ALLOWED, "Method Not Allowed");
	// 		}
	// 		break;
	// 	}
	// }
	// n = write(sock, response.c_str(), strlen(response.c_str()));
	// if (n < 0)
	// 	error("Can't send the response");
	(void)sock;
}

void Worker::error(const char *s)
{
	perror(s);
	exit(1);
}

// Generate a default error page -- TODO: add conf errors page
string Worker::SEND_ERROR(int status, const char *msg)
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
