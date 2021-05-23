#include "Utils.hpp"
#include "Server.hpp"
#include "Headers.hpp"
#include "Config.hpp"
#include "Worker.hpp"

using namespace std;
// static int count_requests = 0;
// static pthread_mutex_t  print_mutex = PTHREAD_MUTEX_INITIALIZER;
void dostuff(int); /* function prototype */

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void *main_loop(void *arg)
{
	Worker *w = reinterpret_cast<Worker *>(arg);
	dprintf(1, "coucou fdp %d!\n", w->getId());
	// w->setStatus(false);

	while (1)
	{
		if (w->getSocket() != 0)
		{
			w->getServer()->log("\e[1;96m[Worker " + to_string(w->getId()) + " is working on this server ! POST Method.]\e[0m");
			w->setSocket(0);
			break;
		}
	}

	return NULL;
}

int main(int argc, char *argv[])
{

	string path;
	if (argc < 2)
		path = "./default/default.conf";
	else
		path = argv[1];

	Config config(path);

	for (int i = 0; i < config.getCountWorkers(); i++)
	{
		config.getWorkers().find(i)->second->setSocket(0);
		pthread_t *thread = config.getWorkers().find(i)->second->getThread();
		pthread_create(thread, NULL, main_loop, reinterpret_cast<void *>(config.getWorkers().find(i)->second));
		usleep(10);
		pthread_detach(*thread);
		pthread_join(*thread, NULL);
	}
	dprintf(1, "miaou miaou %d\n", config.getWorkers().find(0)->second->getStatus());

	//////////////// NEW ! -- fork for each server
	config.run(config.getWorkers(), config.getCountWorkers());
	// for (size_t i = 0; i < config.getServers().size(); i++)
	// {
	// 	// if(!fork())
	// 	config.getAt(i)->run(config.getWorkers(), config.getCountWorkers());
	// 	// else
	// 	cout << "Server [\e[" << 92 + i << ";1m" << i << "\e[0m] launched !" << endl;
	// }

	// for (int i = 0; i < config.getCountWorkers(); i++)
		
	return 0;
}

// void dostuff(int sock)
// {
// 	int n;
// 	ifstream file("default/index.html");
// 	ostringstream text;
// 	text << file.rdbuf();

// 	std::string response1 =
// 		"\n\r" +
// 		text.str();
// 	char buffer[4096];
// 	bzero(buffer, 4096);
// 	n = read(sock, buffer, 4096);
// 	// Test
// 	Headers header;
// 	header += string(buffer);
// 	map<string, string>oui = header.last();
// 	int status_code = 0;
// 	status_code = header.check(oui);
// 	cout << "\e[1;32mRequest #" << count_requests++ << " " << oui.find("Request-Type")->second;
// 	if (oui.find("Request-Type")->second == "POST")
// 		cout << " | [" << oui.find("Content-Length")->second << "].\e[0;0m\n";
// 	else
// 		cout << "\e[0;0m\n";
// 	// if (status_code != STATUS_OK)
// 	// {
// 	// 	//  string response = header.return_response_header(status_code, header);
// 	// 	// n = write(sock, response.c_str(), strlen(response.c_str()));
// 	// 	return ;
// 	// }
// 	if (!dispatcher_type_requests(oui))
// 		;
// 	 string response = header.return_response_header(status_code, header, text.str().size());
// 	 response += response1;
// 	// for(map<string, string>::iterator it = oui.begin(); it != oui.end(); it++)
// 	// 	dprintf(1,"\e[92m%s\e[0m -> |\e[93m%s\e[0m|\n", it->first.c_str(), it->second.c_str());
// 	std::string new_buffer(buffer);

// 	// Fin de test;
// 	if (n < 0)
// 		error("ERROR reading from socket");
// 	//printf("\e[95mHere is the message:\n%s\e[0m\n", buffer);
// 	n = send(sock, response.c_str(), strlen(response.c_str()), MSG_DONTWAIT);
// 	// n = write(sock, response.c_str(), strlen(response.c_str()));
// 		// dprintf(1, "combien tu as print mon coquin ? %d %lu\n", n, strlen(response.c_str()));
// 	// cout << "HEADER RENVOYEE:\n" << response << "\e[0m" << endl;
// 	if (n < 0)
// 		error("ERROR writing to socket_nbet");
// }
