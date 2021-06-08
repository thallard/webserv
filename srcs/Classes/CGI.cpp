#include "CGI.hpp"

CGI::CGI(Server &server, map<string, string> header, Client &client)
{
    (void)server;
    (void)client;
    vector<string> env;
    // Create environment for the CGI
    // env.push_back("REMOTE_HOST=127.0.0.1");
    // env.push_back("SERVER_NAME=" + server.getName());
    // env.push_back("SERVER_PORT=" + to_string(server.getPort()));
    // env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    // env.push_back("SERVER_SOFTWARE=HTTP/1.1");
    // env.push_back("REQUEST_METHOD=" + method);
    env.push_back("PATH_INFO=" + string(getcwd(NULL, 2048)));
    env.push_back("SCRIPT_FILENAME=" + header.find("Location")->second);
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("REQUEST_METHOD=" + header.find("Request-Type")->second);
    env.push_back("REDIRECT_STATUS=200");
    if (header.count("Variables"))
        env.push_back("QUERY_STRING=" + header.find("Variables")->second);

    
    size_t i = 0;
    char **yes = new char *[env.size() + 1];
    cout << "taille d'ici :  " << env.size() << endl;
    for (; i != env.size(); i++)
    {
        cout << "i: " << i << endl;
        yes[i] = (char *)env.at(i).c_str();
    }
    yes[i] = NULL;


    // i = 0;
    // while (yes[i])
    // {
    //     cout << i <<" : " << yes[i] << endl;
    //     i++;
    // }
    _env = yes;
    // prepareFileDescribtors(server, client);
}

CGI::~CGI()
{
}

void CGI::prepareFileDescribtors(Server &server, Client &client)
{
    int pipe_read[2], pipe_write[2], ret = 0;
    pid_t pid;

    string extension;
    string::reverse_iterator it = client.getPath().rbegin();
    for (; it != client.getPath().rend(); it++)
    {
        extension.insert(extension.begin(), *it);
        if (*it == '.')
            break;
    }

    char *info_cgi[3] = {(char *)server.getExtensions().find(extension)->second.find("cgi_path")->second.at(0).c_str(), NULL};

    if (pipe(pipe_read) == -1 || pipe(pipe_write) == -1)
    {
        perror("Pipe failed");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (!pid)
    {
        dup2(pipe_read[0], STDIN_FILENO);
        close(pipe_read[0]);
        close(pipe_read[1]);
        dup2(pipe_write[1], STDOUT_FILENO);
        close(pipe_write[0]);
        close(pipe_write[1]);
        ret = execve(info_cgi[0], info_cgi, _env);
    }
    else
    {
        close(pipe_read[0]);
        int oui = write(pipe_read[1], client.getContent().c_str(), client.getContent().size());

        if (oui <= 0)
        {
            perror("Failed to write from pipe");
            exit(EXIT_FAILURE);
        }
        close(pipe_read[1]);
        char buf[1550];

        int nb = read(pipe_write[0], &buf, 1000);
        if (nb <= 0)
        {
            perror("Failed to read from pipe");
            exit(EXIT_FAILURE);
        }
        dprintf(1, "\e[31mle buf 100 : [%s]\n\e[0m", buf);
        close(pipe_write[0]);
        // close(fd);
        waitpid(pid, NULL, 1);
    }
}