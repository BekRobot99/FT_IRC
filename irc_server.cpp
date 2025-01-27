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
                _accept_new_connection();
            } 
        }
    }
}

void Server::_accept_new_connection() {
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
