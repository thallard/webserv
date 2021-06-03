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
#include <errno.h>
using namespace std;


#define ERROR 0
#define SUCCESS 1

#define STATUS_OK 200
#define STATUS_NO_CONTENT 204

#define STATUS_HEAD 9999

#define STATUS_BAD_REQUEST 400
#define STATUS_METHOD_NOT_ALLOWED 405
#define STATUS_NOT_FOUND 404
#define STATUS_URI_TOO_LONG 414
#define STATUS_TEAPOT 418

#define DEFAULT_PATH "default/default.conf"

#define MAX_WORKERS 50 

#define DEFAULT_PORT 8080


typedef struct s_options
{
	vector<string> methods;
	map<string, string> params;
}				t_options;

typedef struct s_loc
{
	s_loc *parent;
	string path;
	list<s_loc *> childs;
	t_options options;
}				t_loc;

typedef struct _s_preServ
{
	int id;
	int port;
	string name;
	map<int, string> err;
	t_loc *_root;
	map<string, map<string, vector<string> > > ext;
} 			_t_preServ;

typedef struct s_find
{
	t_loc *loc;
	string path;
}			t_find;


int dispatcher_type_requests(map<string, string> request);
void *main_loop(void * arg);

string trim_whitespace(string s);

string get_key(string line);

string get_val(string line);

#endif