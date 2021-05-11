#include "Config.hpp"

Config::Config(char *path)
{
	//int fd;
	(void)path;
	/*fd = open(path, O_RDONLY | O_DIRECTORY);
	if (fd != -1)
	{
		cout << "\e[91mWebServ: " << path << ": is a directory." << endl;
		exit(1);
	}
	close(fd);
	fd = open(path, O_RDONLY);
	if (fd < 3)
	{
		cout << "\e[91mWebServ: " << path << ": canno't open file." << endl;
		exit(1);
	}
	string line;
	ifstream couille(path);
	while (getline(couille, line))
	{
		dprintf(1, "eqweq\n");
		cout << line << endl;
	}
	close(fd);*/
}

Config::~Config(){}