#include "Utils.hpp"
#include "Server.hpp"
#include "Headers.hpp"
#include "Core.hpp"
#include "Worker.hpp"

using namespace std;

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
	socklen_t clilen = sizeof(w->getServer()->getCliAddr());
	while (1)
	{

		if (w->getServer() != NULL)
		{
			w->setStatus(false);
			int newsockfd = accept(w->getServer()->getSocket(), (struct sockaddr *)w->getServer()->getCliAddr_ptr(), &clilen);
			pthread_mutex_lock(w->getServer()->getLogger());
				cout << "\e[1;96m[Worker " << to_string(w->getId()) << "]\e[0m";
				pthread_mutex_unlock(w->getServer()->getLogger());
			w->getServer()->log("oui et socket de la requete :" + to_string(newsockfd));
			// char bouffe[2048];
			// read(w->getServer()->getSocket(), bouffe, 2048);
			// cout << bouffe << endl;
			close(newsockfd);
			w->setServer(NULL);
			w->setStatus(true);
			// cout << "gros chien de la casse\n";
			// Client client;
			// // Find the current client on these available
			// list<Client>::iterator begin = w->getServer()->getClients().begin();
			// while (begin != w->getServer()->getClients().end())
			// {
			// 	if (w->getSocket() == begin->getSocket())
			// 		client = *begin;
			// 	begin++;
			// 	cout << "gros con de mutex" << endl;
			// }
			// cout << "gros con de mutex " << endl;
			// pthread_mutex_lock(w->getServer()->getLogger());
		
			// pthread_mutex_unlock(w->getServer()->getLogger());
			// w->getServer()->handle_request(client);
			// w->setStatus(true);
			// //close(w->getSocket());
			// w->setSocket(0);

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
	return (NULL);
}

int main(int argc, char *argv[])
{
	string path;
	if (argc < 2)
		path = "./default/default.conf";
	else
		path = argv[1];

	Core core(path);

	// Threads for multi-workers
	for (int i = 0; i < core.getCountWorkers(); i++)
	{
		pthread_t *thread = core.getWorkers().find(i)->second->getThread();
		pthread_create(thread, NULL, main_loop, reinterpret_cast<void *>(core.getWorkers().find(i)->second));
		usleep(10);
		pthread_detach(*thread);
		pthread_join(*thread, NULL);
	}

	// Threads for multi-servers
	for (size_t i = 0; i < core.getServers().size(); i++)
	{
		core.setIdServer(i);
		pthread_t *thread = core.getServers().at(i)->getThread();
		pthread_create(thread, NULL, run, reinterpret_cast<void *>(&core));
		usleep(10);
		pthread_detach(*thread);
	}
	while (1)
		;

	return 0;
}