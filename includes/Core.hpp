#ifndef CORE_HPP
#define CORE_HPP
#include "Headers.hpp"
#include "Worker.hpp"
#include "Server.hpp"
#include "Config.hpp"

class Core
{
private:
    Config *config;
    map<int, Worker *> _workers;
    map<int, Server *> _servers;
public:
    Core(/* args */);
    ~Core();

    void run(map<int, Worker *> & workers, int count);
    
};



#endif