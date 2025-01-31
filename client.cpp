#include "Client.hpp"
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

std::map<std::string, Channel*> Client::get_connected_channels() const { // Get the channels the client has joined
    return _joinedChannels;
}

int Client::getSocket() const { // Get the client's file descriptor
    return _fd;
}

private:
    std::string _buffer; // Buffer for stopped clients
    bool _is_stopped;    // Flag to indicate if the client is stopped

    public:
        Client(int fd) : _fd(fd), _is_stopped(false) {}

        void set_stopped(bool value) { _is_stopped = value; }
        bool is_stopped() const { return _is_stopped; }

        void append_to_buffer(const std::string& data) {
            _buffer += data;
        }

        std::string get_buffer() const {
            return _buffer;
        }

        void clear_buffer() {
            _buffer.clear();
        }