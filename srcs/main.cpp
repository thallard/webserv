#include "Utils.hpp"
#include "Server.hpp"

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
			if(FD_ISSET(server.getSocket(), &read_fds2))
               dprintf(1,"READ\n");
			else if(FD_ISSET(server.getSocket(), &write_fds2))
                dprintf(1,"WRITE\n");

			newsockfd = accept(server.getSocket(), (struct sockaddr *)server.getCliAddr_ptr(), &clilen);
			if (newsockfd < 0)
				error("ERROR on accept");
			dostuff(newsockfd);
			close(newsockfd);
		}
		else
			dprintf(1, "No response pendant 10sec\n");
	}
	return 0;
}

void dostuff(int sock)
{
	int n;
	std::ifstream file("default/index.html");
	std::ostringstream text;
	text << file.rdbuf();
	std::string response =
		"HTTP/1.1 200 OK\n"
		"Date: Mon, 27 Jul 2009 12:28:53 GMT\n"
		"Server: Apache/2.2.14 (Win32)\n"
		"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\n"
		"Content-Length:" +
		std::to_string(text.str().size()) +
		"\n"
		"Content-Type: text/html\n"
		"Connection: Closed\n"
		"\n" +
		text.str();
	char buffer[4096];
	bzero(buffer, 4096);
	n = read(sock, buffer, 4096);
	if (n < 0)
		error("ERROR reading from socket");
	printf("\e[95mHere is the message: %s\e[0m\n", buffer);
	n = write(sock, response.c_str(), strlen(response.c_str()));
	// dprintf(1, "combien tu as print mon coquin ? %d %lu", n, strlen(response.c_str()));
	if (n < 0)
		error("ERROR writing to socket_nbet");
}