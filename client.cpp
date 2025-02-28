#include "client.hpp"
#include "channel.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

// Constructors and Operators
Client::Client() : _registrationStatus(STATUS_PASS) {}

Client::Client(int fd) : _fd(fd), _registrationStatus(STATUS_PASS)  {}

Client::~Client() {}

Client::Client(const Client& other)
{
	*this = other;
}

Client& Client::operator= (const Client& other)
{
	if (this != &other)
	{
		_fd = other._fd;

		_clientNickname = other._clientNickname;
		_clientUsername = other._clientUsername;
		_clientRealname = other._clientRealname;

		_registrationStatus = other._registrationStatus;

		_joinedChannels = other._joinedChannels;
		_activeChannel = other._activeChannel;

		_inputBuffer = other._inputBuffer;
		_responseBuffer = other._responseBuffer;
		_outputBuffer = other._inputBuffer;
	}
	return *this;
}

// Getters
int Client::getSocket() const { // Get the client's file descriptor
    return _fd;
}

std::string Client::_obtainNickname() const
{
	return _clientNickname;
}

std::string Client::_obtainUsername() const
{
	return _clientUsername;
}

std::string Client::_obtainRealname() const
{
	return _clientRealname;
}

ClientStatus Client::getRegistrationStatus() const
{
	return _registrationStatus;
}

std::string Client::_obtainHostname() const {
    return _hostname;
}

std::map<std::string, Channel*> Client::get_connected_channels()
{
	return _joinedChannels;
}

std::string Client::obtainInputBuffer()
{
	return _inputBuffer;
}

void Client::updateUsername(std::string& nickname)
{
	_clientNickname = nickname;
}

void Client::storeUsername(std::string& username)
{
	_clientUsername = username;
}

void Client::storeRealname(std::string& realname)
{
	_clientRealname = realname;
}

void Client::queueResponseMessage(std::string buffer)
{
	std::cout << "out - " << _clientNickname <<  ": " << buffer;
	_responseBuffer += buffer;
}

void Client::storeInInputBuffer(char* buffer)
{
    std::cout << "Appending to input buffer: " << buffer << std::endl; // Log the buffer
    _inputBuffer += std::string(buffer);
}

void Client::truncateInputBuffer(size_t pos)
{
    if (pos >= _inputBuffer.length())
    {
        _inputBuffer.clear();
    }
    else
    {
        _inputBuffer.erase(0, pos);
    }
}

bool Client::is_incoming_msg_complete() const
{
	return _inputBuffer.find(std::string("\r\n")) != std::string::npos;
}

bool Client::is_incoming_msg_too_long() const
{
	std::size_t index = _inputBuffer.find(std::string("\r\n"));
	if (index != std::string::npos)
	{
		return index >= (MAX_MSG_SIZE - 1);
	}
	return _inputBuffer.length() > MAX_MSG_SIZE - 2;
}

bool Client::is_response_complete() const
{
	return _responseBuffer.length() != 0;
}


void Client::enterChannel(std::string channelName, Channel *targetChannel)
{
	_joinedChannels[channelName] = targetChannel;
}

short Client::send_out_buffer()
{
	int ret = send(_fd, NULL, 0, SO_NOSIGPIPE);
	if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
	{
		// The writing operation would block
		return 0;
	}
	else if (ret == -1)
	{
		// -1 == disconected client
		return -1;
	}

	_outputBuffer = _responseBuffer;
	_responseBuffer.clear();

	int bytes_sent = send(_fd, _outputBuffer.c_str(), _outputBuffer.length(), SO_NOSIGPIPE);

	if (bytes_sent == -1)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			// -1 == disconected client
			return -1;
		}
	}
	return (0);
}

void Client::updateRegistrationStatus() {
    if (_registrationStatus == STATUS_PASS)
		_registrationStatus = STATUS_NICK;
	else if (_registrationStatus == STATUS_NICK)
		_registrationStatus = STATUS_USER;
	else if (_registrationStatus == STATUS_USER)
		_registrationStatus = STATUS_REGISTERED;
}

bool Client::is_registered() const { // Get the client's registration status
    return _registrationStatus == STATUS_REGISTERED;
}



std::map<std::string, Channel*> Client::getSubscribedChannels() const {
    return _joinedChannels;
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
