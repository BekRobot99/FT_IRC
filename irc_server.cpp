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

// Remove a client from the server
void Server::_deleteClient(int clientFd) {
    close(clientFd);
    _clientsBySocket.erase(clientFd);

    for (std::vector<struct pollfd>::iterator it = _DescriptorsPoll.begin(); it != _DescriptorsPoll.end(); ++it) {
        if (it->fd == clientFd) {
            _DescriptorsPoll.erase(it);
            _numPollDescriptors--;
            break;
        }
    }

    std::cout << "Client disconnected\n";
}
