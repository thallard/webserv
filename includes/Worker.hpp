#ifndef WORKERS_HPP
#define WORKERS_HPP
#include "Utils.hpp"
class Worker
{
private:
	pthread_t *thread;
	bool is_available;
	int socket;
	int id;

public:
	Worker();
	Worker(int id);
	~Worker();
	bool getStatus();
	int getId();
	int getSocket();
	pthread_t *getThread();

	void setStatus(bool);
	void setSocket(int);
};



#endif