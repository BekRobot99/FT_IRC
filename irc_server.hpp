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
#include "Channel.hpp"

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
        std::map<std::string, Channel>	_channelsByName;

    public:
        Server(int port, std::string password);
        ~Server();
        void                   startRun();

    private:
        void					_acceptClientConnection();
        void                   _handleEvents();
        void                   _processClientData(int fd);
        void                    _handleClientDisconnection(Client* client);
        void                    _transmit_to_all_connected_channels(Client* client, const std::string& message);
        void					_distribute_msg_to_channel_members(Client *sender, Channel *channel, const std::string& msg, bool includeSender);
        void					_disconnect_client(Client* user, std::string exitMessage);
        void					_deleteClient(int clientFd);
        void                    _process_command(int clientSocket, const std::string& rawCommand);
        std::vector<std::string> _tokenizeString(const std::string& input, char separator);

        // Commands Handlers 
        void					_handle_pass(Client* user, std::vector<std::string> credentials);
        void					_handle_nick(Client* user, std::vector<std::string> credentials);
};

#endif
