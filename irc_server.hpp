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
        std::vector<std::string>		_registeredUsernames;

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
        void					_handle_user(Client* user, std::vector<std::string> credentials);
        void                    _sendWelcomeMessage(Client* user);
        void                    _handle_ping(Client* user, std::vector<std::string> credentials);
        void                    _handle_join(Client* user, std::vector<std::string> credentials);
        void                   _handle_privmsg(Client* user, std::vector<std::string> credentials);
        Client*                 _locateClientByNickname(const std::string& nickname);
        void                    _handle_quit(Client* user, std::vector<std::string> credentials);
        void                    _handle_who(Client* user, std::vector<std::string> credentials);
        void                    _handle_topic(Client* user, std::vector<std::string> credentials);
        void                    _handle_mode(Client* user, std::vector<std::string> credentials);
        void                   _handle_channel_mode(Client* user, std::vector<std::string> credentials);
        void                   _handle_user_mode(Client* user, std::vector<std::string> credentials);
        void                   _handle_invite(Client* user, std::vector<std::string> credentials);
        void                   _handle_kick(Client* user, std::vector<std::string> credentials);
        void                   _handle_cap(Client* user, std::vector<std::string> credentials);

        // Utils
        bool					_checkNicknameValid(const std::string& nickname);
        bool					_isUsernameTaken(const std::string& username);
        void                   _distributeMessageToChannelMembers(Client* sender, Channel* channel, const std::string& msg, bool includeSender);
        void                   _notifyAllSubscribedChannels(Client* sender, const std::string& message);
};

#endif
