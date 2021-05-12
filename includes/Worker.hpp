#ifndef WORKERS_HPP
#define WORKERS_HPP
#include "Utils.hpp"
class Worker
{
private:
	pthread_t *thread;
	bool is_available;
	int id;

public:
	Worker();
	Worker(int id);
	~Worker();
	bool getStatus();
	int getId();
	pthread_t *getThread();

	void setStatus(bool);
};



#endif