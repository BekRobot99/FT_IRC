#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

// Default constructor
Client::Client() : _fd(-1), _clientNickname(""), _clientUsername(""), _clientRealname(""), _registrationStatus(STATUS_PASS), _activeChannel(nullptr), _hostname(""), _servername("") {}

// Parameterized constructor
Client::Client(int fd) : _fd(fd), _clientNickname(""), _clientUsername(""), _clientRealname(""), _registrationStatus(STATUS_PASS), _activeChannel(nullptr), _hostname(""), _servername("") {}

// Destructor
Client::~Client() {
    if (_fd != -1) {
        close(_fd);
    }
}

// Copy constructor
Client::Client(const Client& other) {
    _hostname = other._hostname;
    _servername = other._servername;
    _fd = other._fd;
    _clientUsername= other._clientUsername;
    _clientNickname= other._clientNickname;
     _joinedChannels = other._joinedChannels;
    _clientRealname= other._clientRealname;
     _inputBuffer = other._inputBuffer;
   _outputBuffer = other._outputBuffer;
    _responseBuffer = other._responseBuffer;
    _activeChannel = other._activeChannel;
}

// Assignment operator
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        _clientNickname= other._clientNickname;
        _servername = other._servername;
        _hostname = other._hostname;
        _clientUsername= other._clientUsername;
       _clientRealname = other._clientRealname;
        _registrationStatus = other._registrationStatus;
        _joinedChannels = other._joinedChannels;
        _activeChannel = other._activeChannel;
       _inputBuffer = other._inputBuffer;
        _outputBuffer = other._outputBuffer;
        _responseBuffer = other._responseBuffer;
        _fd = other._fd;
    }
    return *this;
}

// Getters
std::string Client::_obtainNickname() const { // Get the client's nickname
    return _clientNickname;
}

std::string Client::_obtainUsername() const {
    return _clientUsername;
}

std::string Client::_obtainHostname() const {
    return _hostname;
}

std::string Client::_obtainRealname() const {
    return _clientRealname;
}

std::map<std::string, Channel*> Client::get_connected_channels() const { // Get the channels the client has joined
    return _joinedChannels;
}

int Client::getSocket() const { // Get the client's file descriptor
    return _fd;
}

bool Client::is_registered() const {
    return _registrationStatus == STATUS_REGISTERED;
}

void Client::updateRegistrationStatus(bool isRegistered) {
    _registrationStatus = isRegistered ? STATUS_REGISTERED : STATUS_PASS;
}

void Client::updateUsername(const std::string& newUsername) {
    _clientNickname= newUsername; // Set the client's nickname (ewUsername = new nickname)
}

void    Client::storeUsername(const std::string& username) {
    _clientUsername = username;
}

void    Client::storeRealname(const std::string& realname) {
    _clientRealname = realname;
}

std::map<std::string, Channel*> Client::getSubscribedChannels() const {
    return _joinedChannels;
}

void Client::enterChannel(const std::string& channelName, Channel* targetChannel) {
    _joinedChannels[channelName] = targetChannel;
    _activeChannel = targetChannel;
    targetChannel->addMember(this);
}

