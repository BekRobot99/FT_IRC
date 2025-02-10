#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>
#include <sstream>

// Default constructor
Channel::Channel() : _groupName(""), _accessKey(""), _maxMembers(0), _restrictedAccess(false), _topicLock(false), _discussionTopic("") {}

// Parameterized constructor
Channel::Channel(std::string name) : _groupName(name), _accessKey(""), _maxMembers(0), _restrictedAccess(false), _topicLock(false), _discussionTopic("") {}

// Destructor
Channel::~Channel() {}

// Get the list of members
const std::vector<Client*>& Channel::getMembers() const {
    return _members;
}

// Set the channel password
void Channel::storePassword(const std::string& password) {
    _accessKey = password;
}
