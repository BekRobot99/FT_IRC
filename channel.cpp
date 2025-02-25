#include "channel.hpp"
#include "client.hpp"
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
    for (std::vector<Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it) {
        if (*it == user) {
            return true;
        }
    }
    return false;
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

// remove a moderator
void Channel::removeModerator(const std::string& nickname) {
    std::vector<std::string>::iterator it = std::find(_moderators.begin(), _moderators.end(), nickname);
    if (it != _moderators.end())
        _moderators.erase(it);
}

// add invited user
void Channel::addInvitedUser(Client* user) {
    if (!isUserInvited(user->_obtainNickname())) {
        _guestList.push_back(user->_obtainNickname());
    }
}

// Check if a nickname is invited
bool Channel::isUserInvited(const std::string& nickname) const {
    std::vector<std::string>::const_iterator it = std::find(_guestList.begin(), _guestList.end(), nickname);
    return it != _guestList.end();
}

// Remove a client from the channel
void Channel::removeMember(Client* user) {
    std::vector<Client*>::iterator it = std::find(_members.begin(), _members.end(), user);
    if (it != _members.end()) {
        _members.erase(it);
    }
}

// Check if the channel is at capacity
bool Channel::isAtCapacity() const {
    if (_maxMembers == 0) {
        return false;
    }
    return _members.size() >= _maxMembers;
}

bool Channel::isBlocked(const std::string& nickname) const {
    std::vector<std::string>::const_iterator it = std::find(_blockedUsers.begin(), _blockedUsers.end(), nickname);
    return it != _blockedUsers.end();
}

bool Channel::hasRestrictedAccess() const {
    return _restrictedAccess;
}

std::string Channel::obtain_Password() const {
    return _accessKey;
}

std::string Channel::getMemberList() {
    std::string namesList;
    for (size_t i = 0; i < _members.size(); ++i) {
        Client* client = _members[i];
        std::string prefix = "";
        if (isModerator(client->_obtainNickname())) {
            prefix = "@";
        }
        namesList += prefix + client->_obtainNickname();
        if (i < _members.size() - 1) {
            namesList += " ";
        }
    }
    return namesList;
}

std::string Channel::getDiscussionTopic() const {
    return _discussionTopic;
}

bool Channel::isTopicRestricted() const {
    return _topicLock;
}

void Channel::configureInviteOnly(bool isInviteOnly) {
    _restrictedAccess = isInviteOnly;
}

void Channel::updateTopicRestriction (bool isTopicRestricted) {
    _topicLock = isTopicRestricted;
}

void Channel::setMemberLimit(unsigned short memberLmit) {
    _maxMembers = memberLmit;
}
