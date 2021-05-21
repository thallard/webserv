#include "Utils.hpp"
#include "Server.hpp"
#include "Headers.hpp"
#include "Config.hpp"
#include "Worker.hpp"

using namespace std;
static int count_requests = 0;
static pthread_mutex_t  print_mutex = PTHREAD_MUTEX_INITIALIZER;
void dostuff(int); /* function prototype */


void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void *main_loop(void * arg)
{
	Worker *w = reinterpret_cast<Worker *>(arg);
		// dprintf(1, "coucou fdp %d!\n", w->getId());
	// w->setStatus(false);

	// while (1)
	// {
	// 	w->setStatus(false);
	// 	w->setStatus(true);
	// }
	sleep(1);
	if (!w->getStatus())
	{
		pthread_mutex_lock(&print_mutex);
		w->setStatus(true);
		cout << "\e[1;32mWorker " << w->getId() << " is now available!\e[0;0m\n";
		pthread_mutex_unlock(&print_mutex);
	}
	
	(void)arg;
	return NULL;
}

// struct timeval {
//     time_t         tv_sec;     /* seconds */
//     suseconds_t    tv_usec;    /* microseconds */
// };

int main(int argc, char *argv[])
{
	//int newsockfd;
	//socklen_t clilen;
	
	string path;
	if (argc < 2)
		path = "./default/default.conf";
	else
		path = argv[1];
	

	Config config(path);
		

	//Socket() && IOCTL() && BIND()
	//Server server(8080);
	for (int i = 0; i < config.getCountWorkers(); i++)
	{
		pthread_t *thread = config.getWorkers().find(i)->second->getThread();
		pthread_create(thread, NULL, main_loop, reinterpret_cast<void *>(config.getWorkers().find(i)->second));
		usleep(10);
		pthread_detach(*thread);
		pthread_join(*thread, NULL);
	}
	dprintf(1, "miaou miaou %d\n", config.getWorkers().find(0)->second->getStatus());
	
	// Worker workers(config);
	// cout << "heho je suis sortis de config" << endl;

	//LISTEN()



	// for(int i = 0; i < config.getServers().size(); i++)
	// 	listen(config.getServers()[i].getSocket(), 4096); // TODO Attention ici il faut le monter a 4096 sinon le tester passe pas !

	///clilen = sizeof(server.getCliAddr());

	//FD_ZERO()
	// fd_set write_fds;
	// fd_set read_fds;

	// FD_ZERO(&write_fds);
	// FD_ZERO(&read_fds);

	// //FD_SET()
	// FD_SET(server.getSocket(), &write_fds);
	// FD_SET(server.getSocket(), &read_fds);

	// Timeout

	//////////////// NEW ! -- fork for each server
	for(size_t i = 0; i < config.getServers().size(); i++)
	{
		if(!fork())
			config.getAt(i)->run(config.getWorkers(), config.getCountWorkers());
		else
			cout << "Server [\e[" << 92 + i  << ";1m" << i << "\e[0m] launched !" << endl;
	}
	
	while (1);
///TEST

	/*struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = 3;
	while (1)
	{
		cout << "\e[1;91mnew while\e[0m\n";
		 for (size_t j = 0; j < 2; j++)
		 {
			Server server = *test[j];
			cout << server.getName() << " : " << server.getSocket() << endl;
			
			
			fd_set write_fds2 =  write_fds;
			fd_set read_fds2 =  read_fds;

			//SELECT()
			////////////////// /!\ HERE BG
			if (select(server.getSocket() + 1, &read_fds2, &write_fds2, NULL, &tv) > 0) // thallard : attention ici, peut etre nutiliser quun socket si on a des comportements indefinis
			{
				// Search for an available worker
				int i = 0;
				while (1)
				{		
					i = 0;
					while (!config.getWorkers().find(i)->second->getStatus() && i < config.getCountWorkers() - 1)
						i++;
					if (config.getWorkers().find(i)->second->getStatus())
						break ;
				}
				
				cout << "Worker " << i << " is available!\n";
				if (!config.getWorkers().find(0)->second->getStatus())
					dprintf(1, "\e[0;91mWorker 0 not available!\e[0m\n");
				//FD_ISSET()
				if(!FD_ISSET(server.getSocket(), &read_fds) || !FD_ISSET(server.getSocket(), &write_fds))
					error("ERROR non-set socket");
				newsockfd = accept(server.getSocket(), (struct sockaddr *)server.getCliAddr_ptr(), &clilen);
				if (newsockfd < 0)
					error("ERROR on accept");
				dostuff(newsockfd);
				close(newsockfd);
			}
			else
				dprintf(1, "No response pendant 10sec\n");
				//dprintf(1, "hey!\n");
				// return 1;
				// return 1;
				//dprintf(1, "miaou miaou %d\n", config.getWorkers().find(0)->second->getStatus());
		}
	}*/
	return 0;
}

void dostuff(int sock)
{
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
	cout << "\e[1;32mRequest #" << count_requests++ << " " << oui.find("Request-Type")->second;
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
		error("ERROR writing to socket_nbet");
}
