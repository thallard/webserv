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
	// dprintf(1, "coucou fdp %d!\n", w->getId());
	// w->setStatus(false);

	while (1)
	{
		// pthread_mutex_lock(server->getLogger());
 			
				// pthread_mutex_unlock(server->getLogger());
		if (w->getServer() != NULL && w->getStatus() == false)
		{
			w->setStatus(false);
			cout << "\e[1;96m[Worker " << to_string(w->getId()) << "]\e[0m";
			Server *server = w->getServer();
			bool connection_closed = false, server_end = __FLT16_HAS_QUIET_NAN__;
			socklen_t clilen = sizeof(w->getServer()->getCliAddr());

				pthread_mutex_lock(server->getLogger());
 				cout << "\e[1;96m[Worker " << to_string(w->getId()) << "]\e[0m";
				pthread_mutex_unlock(server->getLogger());

			for (int fd = 0; fd <= server->getMaxSD() && server->getDescReady() > 0; ++fd)
			{
				if (FD_ISSET(fd, server->getTempFD_ptr()))
				{
					server->setDescReady(server->getDescReady() - 1);
					if (fd == server->getSocket())
					{
						cout << "Listening socket is readable!\n";
						int new_socket;
						do {
							new_socket = accept(server->getSocket(), (struct sockaddr *)server->getCliAddr_ptr(), &clilen);
							if (new_socket < 0)
							{
								if (errno != EWOULDBLOCK)
								{
									perror("Accept failed");
									server_end = true;
								}
								break ;
							}
							cout << "OH SA MERE LA CONNECITON SUR LE SOCKET : " << new_socket << endl;
							FD_SET(new_socket, server->getClientsFD_ptr());
								if (new_socket > server->getMaxSD())
									server->setMaxSD(new_socket);
							
						} while (new_socket != -1);
					}
					else
					{
						cout << "Le fd " << fd << " a envie de se faire sauter la gueule\n";
						connection_closed = false;
						int nbytes_read;
						char buffer[4096];
						bzero(buffer, 4096);
						do
						{
							nbytes_read = recv(fd, buffer, 10, 0);
							if (nbytes_read < 0)
							{
								if (errno != EWOULDBLOCK)
								{
									perror("recv() failed\n");
									connection_closed = true;
								}
								break ;
							}

							if (nbytes_read == 0)
							{
								cout << "Connection closed\n";
								connection_closed = true;
									w->setStatus(true);
								break ;
							}
							int len = nbytes_read;
							cout << len << " bytes received\n";
							nbytes_read = send(fd, "bonjour", 7, 0);
							if (nbytes_read < 0)
							{
								perror("error dans le sud\n");
								connection_closed = true;
									w->setStatus(true);
								break ;
							}
						} while (true);
						// close(fd);
						if (connection_closed)
						{
							close(fd);
							FD_CLR(fd, server->getClientsFD_ptr());
							if (fd == server->getMaxSD())
							{
								while (FD_ISSET(server->getMaxSD(), server->getClientsFD_ptr()) == false)
									server->setMaxSD(server->getMaxSD() - 1);
							}
						}
					}
				}
				else 
					cout << "oh mama jai pas trouve le fd la...\n";
				
			}
				w->setStatus(true);
			// if (FD_ISSET(w->getSocket(), server->getClientsFD_ptr()))
			// 	cout << "jexiste deja!\n";
			// else
			// 	FD_SET(w->getSocket(), server->getClientsFD_ptr());
			// w->setStatus(false);
			// int newsockfd = accept(server->getSocket(), (struct sockaddr *)server->getCliAddr_ptr(), &clilen);
			//
			// server->log("oui et socket de la requete :" + to_string(newsockfd));
			
			// char bouffe[2048];
			// read(w->getServer()->getSocket(), bouffe, 2048);
			// cout << bouffe << endl;
			// close(newsockfd);
			w->setServer(NULL);
		
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