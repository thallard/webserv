#include "Utils.hpp"
#include "Server.hpp"
#include "Headers.hpp"

using namespace std;
void dostuff(int); /* function prototype */
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

// struct timeval {
//     time_t         tv_sec;     /* seconds */
//     suseconds_t    tv_usec;    /* microseconds */
// };

int main(int argc, char *argv[])
{
	int newsockfd;
	socklen_t clilen;

	if (argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	//Socket() && IOCTL() && BIND()
	Server server(atoi(argv[1]));

	//LISTEN()
	listen(server.getSocket(), 5);

	clilen = sizeof(server.getCliAddr());

	//FD_ZERO()
	fd_set write_fds;
	fd_set read_fds;

	FD_ZERO(&write_fds);
	FD_ZERO(&read_fds);

	//FD_SET()
	FD_SET(server.getSocket(), &write_fds);
	FD_SET(server.getSocket(), &read_fds);

	// Timeout
	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = 10;
	while (1)
	{
        fd_set write_fds2 = write_fds;
	    fd_set read_fds2 = read_fds;
        //SELECT()
		if (select(server.getSocket() + 1, &read_fds2, &write_fds2, NULL, &tv) > 0)
		{
            //FD_ISSET()
			if(!FD_ISSET(server.getSocket(), &read_fds) || !FD_ISSET(server.getSocket(), &write_fds))
               error("ERROR non-set socket");
			newsockfd = accept(server.getSocket(), (struct sockaddr *)server.getCliAddr_ptr(), &clilen);
			if (newsockfd < 0)
				error("ERROR on accept");
			dostuff(newsockfd);
			close(newsockfd);
		}
		// else
			// dprintf(1, "No response pendant 10sec\n");
	}
	return 0;
}

void dostuff(int sock)
{
	int n;
	ifstream file("default/index.html");
	ostringstream text;
	text << file.rdbuf();
	string response = "";
		// "HTTP/1.1 204 No Content\n"
		// "Date: Mon, 27 Jul 2009 12:28:53 GMT\n"
		// "Server: th-inx\n"
		// "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\n"
		// "Content-Length:" +
		// std::to_string(text.str().size()) +
		// "\n"
		// "Content-Type: text/html\n"
		// "\n";
	char buffer[4096];
	bzero(buffer, 4096);
	n = read(sock, buffer, 4096);
	// Test
	Headers header;
	header += string(buffer);
	map<string, string>oui = header.last();
	int err_code = 0;
	if ((err_code = header.check(oui)) == 1)
	{
		dprintf(1,"Wrong http version\n");
		return ;
	}
	else if (!dispatcher_type_requests(oui))
		;

	for(map<string, string>::iterator it = oui.begin(); it != oui.end(); it++)
		dprintf(1,"\e[92m%s\e[0m -> |\e[93m%s\e[0m|\n", it->first.c_str(), it->second.c_str());

	// Fin de test;
	if (n < 0)
		error("ERROR reading from socket");
	//printf("\e[95mHere is the message:\n%s\e[0m\n", buffer);
	n = write(sock, response.c_str(), strlen(response.c_str()));
	// dprintf(1, "combien tu as print mon coquin ? %d %lu", n, strlen(response.c_str()));
	if (n < 0)
		error("ERROR writing to socket_nbet");
}
