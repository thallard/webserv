#include "Utils.hpp"
#include "Server.hpp"
#include "Headers.hpp"
#include "Core.hpp"
#include "Worker.hpp"

using namespace std;
// static int count_requests = 0;
// static pthread_mutex_t  print_mutex = PTHREAD_MUTEX_INITIALIZER;
void dostuff(int); /* function prototype */

Core *setCore(Core *obj)
{
	static Core *core = {NULL};
	if (obj)
		core = obj;
	return core;
}

Core *getCore()
{
	return setCore(NULL);
}


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
		pthread_mutex_lock(w->getServer()->getLogger());
		cout << "\e[1;96m[Worker " << to_string(w->getId()) << "]\e[0m";
		pthread_mutex_unlock(w->getServer()->getLogger());
		w->getServer()->handle_request(w->getSocket());
		w->setStatus(true);
			//close(w->getSocket());
			w->setSocket(0);
		
			// cout << "allo19\n";
			// break;
		}
	}

	return NULL;
}

void *run(void *arg)
{
	Core *core = reinterpret_cast<Core *>(arg);
	core->getServers().at(core->getIdServer())->run(core->getWorkers());
	return NULL;
}

void launch(Core *core)
{
	for (int i = 0; i < core->getCountWorkers(); i++)
	{

		core->getWorkers().find(i)->second->setSocket(0);
		pthread_t *thread = core->getWorkers().find(i)->second->getThread();
		pthread_create(thread, NULL, main_loop, reinterpret_cast<void *>(core->getWorkers().find(i)->second));
		usleep(10);
		pthread_detach(*thread);
		pthread_join(*thread, NULL);
	}
	dprintf(1, "miaou miaou %d\n", core->getWorkers().find(0)->second->getStatus());

	// Threads for multi-servers
	for (size_t i = 0; i < core->getServers().size(); i++)
	{
		core->setIdServer(i);
		pthread_t *thread = core->getServers().at(i)->getThread();
		pthread_create(thread, NULL, run, reinterpret_cast<void *>(&core));
		usleep(10);
		pthread_detach(*thread);
	}
}

void handle_signal(int sig)
{
	(void)sig;
	cout << "\e[91;1m[CRASHED]" << endl << "\e[92;1m[Restarting ...]\e[0m" << endl;
	launch(getCore());
}

int main(int argc, char *argv[])
{

	//signal(SIGABRT, handle_signal);

	string path;
	if (argc < 2)
		path = "./default/default.conf";
	else
		path = argv[1];

	Core core(path);
	setCore(&core);

	for (int i = 0; i < core.getCountWorkers(); i++)
	{

		core.getWorkers().find(i)->second->setSocket(0);
		pthread_t *thread = core.getWorkers().find(i)->second->getThread();
		pthread_create(thread, NULL, main_loop, reinterpret_cast<void *>(core.getWorkers().find(i)->second));
		usleep(10);
		pthread_detach(*thread);
		pthread_join(*thread, NULL);
	}
	dprintf(1, "miaou miaou %d\n", core.getWorkers().find(0)->second->getStatus());

	// Threads for multi-servers
	for (size_t i = 0; i < core.getServers().size(); i++)
	{
		core.setIdServer(i);
		pthread_t *thread = core.getServers().at(i)->getThread();
		pthread_create(thread, NULL, run, reinterpret_cast<void *>(&core));
		usleep(10);
		pthread_detach(*thread);
	}
	//launch(getCore());
	while (1)
		;
	//////////////// NEW ! -- fork for each server
	//Core.run(Core.getWorkers(), core.getCountWorkers());
	// for (size_t i = 0; i < Core.getServers().size(); i++)
	// {
	// 	// if(!fork())
	// 	Core.getAt(i)->run(Core.getWorkers(), Core.getCountWorkers());
	// 	// else
	// 	cout << "Server [\e[" << 92 + i << ";1m" << i << "\e[0m] launched !" << endl;
	// }

	// for (size_t i = 0; i < core.getServers().size(); i++)
	// 	pthread_join(*core.getServers().at(i)->getThread(), NULL);

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
