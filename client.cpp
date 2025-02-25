#include "client.hpp"
#include "channel.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

// Default constructor
Client::Client() 
    : _fd(-1), 
      _clientUsername(""), 
      _clientNickname(""), 
      _joinedChannels(), 
      _activeChannel(nullptr), 
      _clientRealname(""), 
      _inputBuffer(""), 
      _outputBuffer(""), 
      _responseBuffer(""), 
      _registrationStatus(STATUS_PASS), 
      _hostname(""), 
      _servername(""), 
      _buffer(""), 
      _is_stopped(false), 
      _is_invisible(false), 
      _is_operator(false) {}

// Parameterized constructor
Client::Client(int fd) 
    : _fd(fd), 
      _clientUsername(""), 
      _clientNickname(""), 
      _joinedChannels(), 
      _activeChannel(nullptr), 
      _clientRealname(""), 
      _inputBuffer(""), 
      _outputBuffer(""), 
      _responseBuffer(""), 
      _registrationStatus(STATUS_PASS), 
      _hostname(""), 
      _servername(""), 
      _buffer(""), 
      _is_stopped(false), 
      _is_invisible(false), 
      _is_operator(false) {}

// Destructor
Client::~Client() {
    if (_fd != -1) {
        close(_fd);
    }
}

// Copy constructor
Client::Client(const Client& other) {
    _fd = other._fd;
    _clientUsername = other._clientUsername;
    _clientNickname = other._clientNickname;
    _clientRealname = other._clientRealname;
    _hostname = other._hostname;
    _servername = other._servername;
    _registrationStatus = other._registrationStatus;
    _activeChannel = other._activeChannel;
    _joinedChannels = other._joinedChannels;
    _inputBuffer = other._inputBuffer;
    _outputBuffer = other._outputBuffer;
    _responseBuffer = other._responseBuffer;
    _buffer = other._buffer;
    _is_stopped = other._is_stopped;
    _is_invisible = other._is_invisible;
    _is_operator = other._is_operator;
}

// Assignment operator
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        _fd = other._fd;
        _clientUsername = other._clientUsername;
        _clientNickname = other._clientNickname;
        _clientRealname = other._clientRealname;
        _hostname = other._hostname;
        _servername = other._servername;
        _registrationStatus = other._registrationStatus;
        _activeChannel = other._activeChannel;
        _joinedChannels = other._joinedChannels;
        _inputBuffer = other._inputBuffer;
        _outputBuffer = other._outputBuffer;
        _responseBuffer = other._responseBuffer;
        _buffer = other._buffer;
        _is_stopped = other._is_stopped;
        _is_invisible = other._is_invisible;
        _is_operator = other._is_operator;
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

bool Client::is_registered() const { // Get the client's registration status
    return _registrationStatus == STATUS_REGISTERED;
}

void Client::updateRegistrationStatus() {
    if (_registrationStatus == STATUS_PASS)
		_registrationStatus = STATUS_NICK;
	else if (_registrationStatus == STATUS_NICK)
		_registrationStatus = STATUS_USER;
	else if (_registrationStatus == STATUS_USER)
		_registrationStatus = STATUS_REGISTERED;
}

void Client::updateUsername(const std::string& newUsername) {
    _clientNickname = newUsername; // Set the client's nickname (newUsername = new nickname)
}

void Client::storeUsername(const std::string& username) {
    _clientUsername = username;
}

void Client::storeRealname(const std::string& realname) {
    _clientRealname = realname;
}

std::map<std::string, Channel*> Client::getSubscribedChannels() const {
    return _joinedChannels;
}

void Client::enterChannel(const std::string& channelName, Channel *targetChannel) {
    _joinedChannels[channelName] = targetChannel;
    _activeChannel = targetChannel;
    targetChannel->addMember(this);
}

void Client::exitChannel(const std::string& channelName) {
    std::map<std::string, Channel*>::iterator it = _joinedChannels.find(channelName);
    if (it != _joinedChannels.end()) {
        it->second->removeMember(this);
        _joinedChannels.erase(it);
        if (_activeChannel == it->second) {
            _activeChannel = nullptr;
        }
    }
}

// Queue a response message
void Client::queueResponseMessage(std::string message) {
    std::cout << "out - " << _clientNickname << ": " << message;
    _responseBuffer += message;
}

ClientStatus Client::getRegistrationStatus() const {
    return _registrationStatus;
}
