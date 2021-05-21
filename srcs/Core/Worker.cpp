#include "Worker.hpp"

Worker::Worker()
{
	is_available = true;
	
	thread = new pthread_t;
	
	// for (int i = 0; i < nb_workers; i++)
	// {
	// 	;
	// }
	// thread = new pthread_t[5];
	// for (int i = 0; i < nb_workers; i++)
	// {
	// 	pthread_create(&thread, NULL, main_loop, this);
	// }
	

}


Worker::Worker(int id)
{
	this->id = id;
	is_available = true;	
	thread = new pthread_t;
}

Worker::~Worker()
{
	dprintf(1, "destructor %d\n", getId());
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

pthread_t *Worker::getThread()
{
	return (thread);
}

void Worker::setStatus(bool val)
{
	is_available = val;
}