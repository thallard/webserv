#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <map>
#include <vector>
#include <list>
#include <pthread.h>
using namespace std;


#define ERROR 0
#define SUCCESS 1

#define STATUS_OK 200
#define STATUS_NO_CONTENT 204

#define STATUS_HEAD 9999

#define STATUS_METHOD_NOT_ALLOWED 405
#define STATUS_NOT_FOUND 404
#define STATUS_TEAPOT 418

#define DEFAULT_PATH "default/default.conf"

#define MAX_WORKERS 50 

#define DEFAULT_PORT 8080

typedef struct _s_preServ
{
	int id;
	int port;
	string name;
	string root;
	map<int, string> err;
	map<string, map<string, string> > loc;
	string index;
	map<string, vector<string> > methods;
	vector<string> allowed;
} 			_t_preServ;

int dispatcher_type_requests(map<string, string> request);
void *main_loop(void * arg);


#endif