#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <ctime>
#include <poll.h>
#include <arpa/inet.h>
#include "Client.hpp"

class Server
{
    private:
        std::string						_serverName;
        int								_serverPort;
        std::string						_serverPassword;
        
        std::string						_serverCeationTime;

        int								_listenerSocket;
        unsigned short					_numPollDescriptors;
        std::vector<struct fdpoll>		_DescriptorsPoll;
        std::map<int, Client>			_clientsBySocket;

    public:
        Server(int port, std::string password);
        ~Server();
        void                   startRun();

    private:
        void					_acceptClientConnection();
        void                   _handleEvents();
        void                   _processClientData(int fd);
        void                    _handleClientDisconnection(Client* client);
};

#endif
