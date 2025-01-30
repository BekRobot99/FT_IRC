#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <string>
#include <set>
#include "Client.hpp"

class Channel
{
private:
    std::string                     _groupName;
    std::vector<Client*>            _members;
    std::vector<std::string>        _moderators;
    std::vector<std::string>        _blockedUsers;

    std::string                     _accessKey;
    unsigned short                  _maxMembers;

    bool                            _restrictedAccess;
    std::vector<std::string>        _guestList;

    bool                            _topicLock;
    std::string                     _discussionTopic;

public:
    Channel();
    Channel(std::string name);
    ~Channel();

    const std::vector<Client*>&      getMembers() const;
};
#endif