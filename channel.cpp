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

// Add a client to the channel
void Channel::addMember(Client* user) {
    if (!hasMember(user)) {
        _members.push_back(user);
    }
}

// Check if a client is in the channel
bool Channel::hasMember(Client* user) const {
    return std::find(_members.begin(), _members.end(), user) != _members.end();
}

// get topic of the channel
std::string Channel::obtainTopic() const {
    return _discussionTopic;
}

// Update the discussion topic
void Channel::updateDiscussionTopic(const std::string& newTopic) {
    _discussionTopic = newTopic;
}

// Check if a nickname belongs to a moderator (operator)
bool Channel::isModerator(const std::string& nickname) const {
    return std::find(_moderators.begin(), _moderators.end(), nickname) != _moderators.end();
}

// Assign a moderator to the channel
void Channel::assignModerator(const std::string& username) {
    if (!isModerator(username)) {
        _moderators.push_back(username);
    }
}

// add invited user
void Channel::addInvitedUser(Client* user) {
    if (!isUserInvited(user->_obtainNickname())) {
        _guestList.push_back(user->_obtainNickname());
    }
}

// Check if a nickname is invited
bool Channel::isUserInvited(const std::string& nickname) const {
    return std::find(_guestList.begin(), _guestList.end(), nickname) != _guestList.end();
}

// Remove a client from the channel
void Channel::removeMember(Client* user) {
    std::vector<Client*>::iterator it = std::find(_members.begin(), _members.end(), user);
    if (it != _members.end()) {
        _members.erase(it);
    }
}
