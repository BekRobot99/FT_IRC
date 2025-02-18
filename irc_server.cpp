#include "Server.hpp"
#include <map>
#include <vector>

Server::Server(int sereverPort, std::string serverPassword, std::string serverName)
    : _sereverPort(port), _serverPassword(serverPassword), _serverName(serverName), _listenerSocket(-1), _numPollDescriptors(0) {
    std::time_t now = std::time(0);
    _serverCeationTime = std::ctime(&now);

    _setup_server_socket();
}

Server::~Server() {
    // Close all client connections
    for (std::map<int, Client>::iterator it = _clientsBySocket.begin(); it != _clientsBySocket.end(); ++it) {
        close(it->first);
    }
    close(_listenerSocket);
}

// Set up the server socket
void Server::_setup_server_socket() {
    _listenerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenerSocket < 0) {
        std::cerr << "Error: Could not create socket\n";
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(_listenerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error: Could not set socket options\n";
        exit(EXIT_FAILURE);
    }

    // Bind the socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_serverPort);

    if (bind(_listenerSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not bind socket\n";
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(_listenerSocket, SOMAXCONN) < 0) {
        std::cerr << "Error: Could not listen on socket\n";
        exit(EXIT_FAILURE);
    }

    // Add the listener socket to the pollfd vector
    struct pollfd listenerPollFd;
    listenerPollFd.fd = _listenerSocket;
    listenerPollFd.events = POLLIN;
    _DescriptorsPoll.push_back(listenerPollFd);
    _numPollDescriptors++;

    std::cout << "Server is listening on port " << _serverPort << "\n";
}

void Server::startRun() {
    while (true) {
        _handleEvents();
    }
}

void Server::_handleEvents() {
    if (poll(_DescriptorsPoll.data(), _numPollDescriptors, -1) < 0) {
        std::cerr << "Error: Poll failed\n";
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < _numPollDescriptors; ++i) {
        if (_DescriptorsPoll[i].revents & POLLIN) {
            if (_DescriptorsPoll[i].fd == _listenerSocket) {
                _acceptClientConnection();
            }
            else {
                _processClientData(_DescriptorsPoll[i].fd);
            }
        }
    }
}

void Server::_acceptClientConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(_listenerSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error: Could not accept connection\n";
        return;
    }

    // Set the client socket to non-blocking mode
    fcntl(clientSocket, F_SETFL, O_NONBLOCK);

    // Add the new client socket to the fdpoll vector
    struct fdpoll clientPollFd;
    clientPollFd.fd = clientSocket;
    clientPollFd.events = POLLIN;
    _DescriptorsPoll.push_back(clientPollFd);
    _numPollDescriptors++;

    // Create a new Client object and add it to the map
    Client newClient(clientSocket);
    _clientsBySocket[clientSocket] = newClient;

    std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "\n";
}

// Parse incoming data from a client
void Server::_processClientData(int fd) {
    
    char buffer[1024];
    ssize_t bytesRead = recv(fd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        _handleClientDisconnection(&_clientsBySocket[fd]);
        return;
    }

    buffer[bytesRead] = '\0';
    _clientsBySocket[fd].append_to_buffer(buffer);

    if (_clientsBySocket[fd].is_stopped()) {
        // Client is stopped; buffer the data and return
        return;
    }

    std::string& clientBuffer = _clientsBySocket[fd].get_buffer();
    size_t pos;
    while ((pos = clientBuffer.find('\n')) != std::string::npos) {
        std::string message = clientBuffer.substr(0, pos);
        clientBuffer.erase(0, pos + 1);

        // Process the complete command
        _process_command(fd, message);
    }
}

void Server::_process_command(int clientSocket, const std::string& rawCommand) {
    std::vector<std::string> commandParts = _tokenizeString(rawCommand, ' ');
    if (commandParts.empty()) return;

    std::string commandName = commandParts[0];
    std::vector<std::string> commandArgs(commandParts.begin() + 1, commandParts.end());

    // Handle the command
    if (commandName == "PASS") {
        _handle_pass(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "NICK") {
        _handle_nick(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "USER") {
        _handle_user(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "PING") {
        _handle_ping(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "JOIN") {
        _handle_join(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "PRIVMSG") {
        _handle_privmsg(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "QUIT") {
        _handle_quit(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "WHO") {
        _handle_who(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "TOPIC") {
        _handle_topic(&_clientsBySocket[clientSocket], commandArgs);
    } else if (commandName == "MODE") {
        _handle_mode(&_clientsBySocket[clientSocket], commandArgs);
    } 
}
     

// Handle PASS command
void Server::_handle_pass(Client* user, std::vector<std::string> credentials) {
    if (credentials.empty()) {
        send(user->getSocket(), "ERROR :Not enough parameters (PASS)\r\n", 36, 0);
        return;
    }

    if (user->is_registered()) {
        send(user->getSocket(), "ERROR :You may not reregister\r\n", 30, 0);
        return;
    }

    if (credentials[0] != _serverPassword) {
        send(user->getSocket(), "ERROR :Password incorrect\r\n", 26, 0);
        return;
    }
    user->storePassword(credentials[0]);
}

// Handle NICK command
void Server::_handle_nick(Client* user, std::vector<std::string> credentials) {
    if (credentials.empty()) {
        send(user->getSocket(), "ERROR : No nickname given\r\n", 26, 0);
        return;
    }

    std::string nickname = credentials[0];
    if (!_checkNicknameValid(nickname)) {
        send(user->getSocket(), "ERROR : Erroneous nickname\r\n", 27, 0);
        return;
    }

    if (_isUsernameTaken(nickname)) {
        send(user->getSocket(), "ERROR : Nickname is already in use\r\n", 35, 0);
        return;
    }

    user->updateUsername(nickname);
    _registeredUsernames.push_back(nickname);

    if (user->is_registered()) {
        send(user->getSocket(), "ERROR : Broadcast nickname change to all channels the client is in\r\n", 67, 0);
        _notifyAllSubscribedChannels(user, "NICK " + nickname);
    }
}

// Handle USER command
void Server::_handle_user(Client* user, std::vector<std::string> credentials) {
    if (credentials.size() < 4) {
        send(user->getSocket(), "ERROR : Not enough parameters\r\n", 30, 0);
        return;
    }

    if (user->is_registered()) {
        send(user->getSocket(), "ERROR : You may not reregister\r\n", 30, 0);
        return;
    }

    user->storeUsername(credentials[0]); // set the username of the client by ali   
    user->storeRealname(credentials[3]); // set the realname of the client by ali

    if (!user->_obtainNickname().empty() && !user->_obtainUsername().empty()) // check if the nickname and username are not empty by ali
    {
        user->updateRegistrationStatus(true); // set the registration status of the client by ali
        _sendWelcomeMessage(user);
    }
}

// Handle PING command
void Server::_handle_ping(Client* user, std::vector<std::string> credentials) {
    if (credentials.empty()) {
        send(user->getSocket(), "ERROR :No origin specified (PING)\r\n", 36, 0);
        return;
    }

    std::string origin = credentials[0];
    std::string pongMessage = ":" + _serverName + " PONG "  + origin + "\r\n";
    send(user->getSocket(), pongMessage.c_str(), pongMessage.size(), 0);
}

// Handle JOIN command
void Server::_handle_join(Client* user, std::vector<std::string> credentials) {
    if (credentials.empty()) {
        send(user->getSocket(), "ERROR : No channel specified\r\n", 30, 0);
        return;
    }

    std::string channelName = credentials[0];
    if (_channelsByName.find(channelName) == _channelsByName.end()) {
        // Create a new channel
        Channel newChannel(channelName);
        _channelsByName[channelName] = newChannel;
    }

    Channel* channel = &_channelsByName[channelName];
    channel->addMember(user);
    user->enterChannel(channelName);

    std::string joinMessage = ":" + user->_obtainNickname() + " JOIN " + channelName + "\r\n";
    _distributeMessageToChannelMembers(user, channel, joinMessage, true);
}

// Handle PRIVMSG command
void Server::_handle_privmsg(Client* user, std::vector<std::string> credentials) {
    if (credentials.size() < 2) {
        send(user->getSocket(), "ERROR : No recipient or message specified\r\n", 43, 0);
        return;
    }

    std::string target = credentials[0];
    std::string message = credentials[1];

    if (target[0] == '#') {
        // Send message to a channel
        if (_channelsByName.find(target) == _channelsByName.end()) {
            send(user->getSocket(), "ERROR : No such channel\r\n", 24, 0);
            return;
        }

        Channel* channel = &_channelsByName[target];
        _distributeMessageToChannelMembers(user, channel, message, false);
    }

    else {
        // Send message to a user
        Client* targetClient = _locateClientByNickname(target);
        if (!targetClient) {
            // send error message
            send(user->getSocket(), "ERROR : No such nick\r\n", 20, 0);
            return;
        }
        std::string privmsg = ":" + user->_obtainNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        send(targetClient->getSocket(), privmsg.c_str(), privmsg.size(), 0);
    }

}

// Handle QUIT command
void Server::_handle_quit(Client* user, std::vector<std::string> credentials) {
    std::string exitMessage = ":" + user->_obtainNickname() + " QUIT :";
    if (!credentials.empty()) {
        exitMessage += credentials[0];
    }
    else {
        exitMessage += "Client disconnected";
    }
    _disconnectClient(user, exitMessage);
}

// Handle WHO command
void Server::_handle_who(Client* user, const std::vector<std::string>& credentials) {
    if (credentials.empty()) {
        send(user->getSocket(), "ERROR : No mask specified\r\n", 26, 0);
        return;
    }

    std::string mask = credentials[0];
    if (mask[0] == '#') {
        // List users in a channel
        if (_channelsByName.find(mask) == _channelsByName.end()) {
            send(user->getSocket(), "ERROR : No such channel\r\n", 24, 0);
            return;
        }

        Channel* channel = &_channelsByName[mask];
        std::vector<Client*> members = channel->getMembers();
        for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
            std::string whoMessage = ":" + _serverName + " 352 " + user->_obtainNickname() + " " + mask + " " + (*it)->_obtainUsername() + " " + (*it)->_obtainHostname() + " " + _serverName + " " + (*it)->_obtainNickname() + " H :0 " + (*it)->_obtainRealname() + "\r\n";
            send(user->getSocket(), whoMessage.c_str(), whoMessage.size(), 0);
        }
    }
    else {
        // List users matching a nickname mask (will be  implemented after by ramez)
    }
}

// Handle TOPIC command
void Server::_handle_topic(Client* user, const std::vector<std::string>& credentials) {
    if (credentials.empty()) {
        send(user->getSocket(), "ERROR : No channel specified\r\n", 30, 0);
        return;
    }

    std::string channelName = credentials[0];
    if (_channelsByName.find(channelName) == _channelsByName.end()) {
        send(user->getSocket(), "ERROR : No such channel\r\n", 24, 0);
        return;
    }

    Channel* channel = &_channelsByName[channelName];
    if (credentials.size() == 1) {
        std::string topic = channel->obtainTopic();
        std::string topicMessage = ":" + _serverName + " 332 " + user->_obtainNickname() + " " + channelName + " :" + topic + "\r\n";
        send(user->getSocket(), topicMessage.c_str(), topicMessage.size(), 0);
    }
    else {
        if (!channel->isModerator(user)) {
            send(user->getSocket(), "ERROR : You're not a channel operator\r\n", 38, 0);
            return;
        }

        std::string newTopic = credentials[1];
        channel->updateDiscussionTopic(newTopic);

        std::string topicMessage = ":" + user->_obtainNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
        _distributeMessageToChannelMembers(user, channel, topicMessage, true);
    }
}

// Handle MODE command
void Server::_handle_mode(Client* user, const std::vector<std::string>& credentials) {
    if (credentials.size() < 2) {
        send(user->getSocket(), "ERROR : Not enough parameters\r\n", 30, 0);
        return;
    }

    std::string target = credentials[0];
    if (target[0] == '#') {
        _handle_channel_mode(user, credentials);
    } else {
        _handle_user_mode(user, credentials);
        send(user->getSocket(), "ERROR : User modes not supported\r\n", 32, 0);
    }
}

std::vector<std::string> Server::_tokenizeString(const std::string& input, char separator) {
    std::vector<std::string> result;
    std::string fragment;
    std::istringstream stream(input);
    
    while (std::getline(stream, fragment, separator)) {
        result.push_back(fragment);
    }
    
    return result;
}


void Server::_handleClientDisconnection(Client* client) {
    std::string exitMessage = ":" + client->_obtainNickname() + " QUIT :Unexpected disconnection";
    _disconnectClient(client, exitMessage);
}

// check from here
// Disconnect a client
void Server::_disconnectClient(Client* user, std::string exitMessage) {
    _transmit_to_all_connected_channels(user, exitMessage);
    _deleteClient(user->getSocket());
}

// Broadcast a message to all channels a client has joined
void Server::_transmit_to_all_connected_channels(Client* user, const std::string& msg) {
    std::vector<std::string> channels = user->get_connected_channels();
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
        Channel* channel = &_channelsByName[*it];
        _distribute_msg_to_channel_members(user, channel, msg, false);
    }
}

// Send a message to all members of a channel
void Server::_distribute_msg_to_channel_members(Client* sender, Channel* channel, const std::string& msg, bool includeSender) {
    std::vector<Client*> members = channel->getMembers();
    for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if (includeSender || (*it)->getSocket() != sender->getSocket()) {
            send((*it)->getSocket(), msg.c_str(), msg.size(), 0);
        }
    }
}

// Check if a nickname is valid
bool Server::_checkNicknameValid(const std::string& nickname) {
    if (nickname.empty() || nickname.size() > 9) {
        return false;
    }

    for (std::string::const_iterator it = nickname.begin(); it != nickname.end(); ++it) {
        if (!isalnum(*it) && *it != '_' && *it != '-') {
            return false;
        }
    }
    return true;
}

// Check if a username is already in use
bool Server::_isUsernameTaken(const std::string& username) {
    for (std::vector<std::string>::iterator it = _registeredUsernames.begin(); it != _registeredUsernames.end(); ++it) {
        if (*it == username) {
            return true;
        }
    }
    return false;
}

// Broadcast a message to all channels a client has joined
void Server::_notifyAllSubscribedChannels(Client* sender, const std::string& message) {
    std::vector<std::string> subscribedChannels = sender->getSubscribedChannels(); // get the channels the client has joined must done in the client class by ali
    for (std::vector<std::string>::iterator it = subscribedChannels.begin(); it != subscribedChannels.end(); ++it) {
        Channel* currentChannel = &_channelsByName[*it];
        _distributeMessageToChannelMembers(sender, currentChannel, message, false);
    }
}

// Send a message to all members of a channel
void Server::_distributeMessageToChannelMembers(Client* sender, Channel* channel, const std::string& msg, bool includeSender) {
    std::vector<Client*> members = channel->getMembers();
    for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if (includeSender || (*it)->getSocket() != sender->getSocket()) {
            send((*it)->getSocket(), msg.c_str(), msg.size(), 0);
        }
    }
}

//welcome new user
void Server::_sendWelcomeMessage(Client* user) {
    std::string wlcmMsg = "Welcome to the " + _serverName +  + " IRC server, " + user->_obtainNickname() + "!\r\n";
    send(user->getSocket(), wlcmMsg.c_str(), wlcmMsg.size(), 0);
}

// Find a client by nickname
Client* Server::_locateClientByNickname(const std::string& nickname) {
    for (std::map<int, Client>::iterator it = _clientsBySocket.begin(); it != _clientsBySocket.end(); ++it) {
        if (it->second._obtainNickname() == nickname) {
            return &it->second;
        }
    }
    return NULL;
}

// handle channel mode
void Server::_handle_channel_mode(Client* user, const std::vector<std::string>& credentials) {
    
    if (credentials.size() < 2) {
        send(user->getSocket(), "ERROR : Not enough parameters (MODE)\r\n", 36, 0);
         return;
    }

    std::string channelName = credentials[0];
    std::string mode = credentials[1];

    if (_channelsByName.find(channelName) == _channelsByName.end()) {
        send(user->getSocket(), "ERROR : No such channel\r\n", 24, 0);
        return;
    }
    
    Channel* channel = &_channelsByName[channelName];
    if (!channel->isModerator(user)) {
        send(user->getSocket(), "ERROR : You're not a channel operator\r\n", 38, 0);
        return;
    }

   //Handle channel modes (+o, +k)
    if (mode == "+o") {
        if (credentials.size() < 3) {
            send(user->getSocket(), "ERROR : Not enough parameters (MODE +o)\r\n", 40, 0);
            return;
        }
        std::string targetNickname = credentials[2];
        Client* targetClient = _locateClientByNickname(targetNickname);
        if (!targetClient || !channel->hasMember(targetClient)) {
            send(user->getSocket(), "ERROR : User not in channel\r\n", 28, 0);
            return;
        }
        channel->assignModerator(targetClient); // will be tested and implemented in the channel class
        std::string modeMessage = ":" + user->_obtainNickname() + " MODE " + channelName + " +o " + targetNickname + "\r\n";
        _distributeMessageToChannelMembers(user, channel, modeMessage, true);
    } else {
        send(user->getSocket(), "ERROR : Unsupported mode\r\n", 26, 0);
    }
    //other cases will be handled after 
}

// Handle user mode
void Server::_handle_user_mode(Client* user, const std::vector<std::string>& credentials) {
    if (credentials.size() < 2) {
        send(user->getSocket(), "ERROR : Not enough parameters (MODE)\r\n", 36, 0);
        return;
    }

    std::string nickname = credentials[0];
    std::string channelName = credentials[1];

    // Check if the target nickname matches the client's nickname
    if (nickname != user->_obtainNickname()) {
        send(user->getSocket(), "ERROR :Cannot change modes for other users\r\n", 42, 0);
        return;
    }

    // Handle setting/unsetting modes
    if (mode.size() < 2 || (mode[0] != '+' && mode[0] != '-')) {
        send(user->getSocket(), "ERROR :Invalid mode format\r\n", 28, 0);
        return;
    }

   char modeChar = mode[1]; // The mode character (e.g., 'i', 'o')
   bool setMode = (mode[0] == '+'); // True if setting the mode, false if unsetting

    switch (modeChar) {
        case 'i': // Invisible mode
            user->set_invisible(setMode);
            send(user->getSocket(), (":" + _serverName + " MODE " + user->_obtainNickname() + " " + mode + "\r\n").c_str(), 0);
            break;

        case 'o': // Operator mode
            if (setMode) {
                // Only the server can grant operator status
                send(user->getSocket(), "ERROR :Cannot set operator mode\r\n", 32, 0);
            } else {
                user->set_operator(false);
                send(user->getSocket(), (":" + _serverName + " MODE " + user->_obtainNickname() + " " + mode + "\r\n").c_str(), 0);
            }
            break;

        default:
            send(user->getSocket(), "ERROR :Unknown mode\r\n", 22, 0);
            break;
    }
}

// Remove a client from the server
void Server::_deleteClient(int clientFd) {
    close(clientFd);
    _clientsBySocket.erase(clientFd);

    for (std::vector<struct fdpoll>::iterator it = _DescriptorsPoll.begin(); it != _DescriptorsPoll.end(); ++it) {
        if (it->fd == clientFd) {
            _DescriptorsPoll.erase(it);
            _numPollDescriptors--;
            break;
        }
    }

    std::cout << "Client disconnected\n";
}
