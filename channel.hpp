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

    void                            addMember(Client* user);
    bool                            hasMember(Client* user) const;
    const std::vector<Client*>&      getMembers() const; // Get the list of clients in the channel
    void                            storePassword(const std::string& password); // Set the channel password
    std::string                     obtainTopic() const; // Get the channel topic
    bool                            isModerator(const std::string& nickname) const; // Check if a nickname belongs to a moderator (operator)
    void                            updateDiscussionTopic(const std::string& newTopic); // Update the discussion topic
    void                             assignModerator(const std::string& username); // Assign a moderator to the channel
    void                            addInvitedUser(Client* user); // Add an invited user
    bool                            isUserInvited(const std::string& nickname) const; // Check if a nickname is invited
    void                            removeMember(Client* user); // Remove a client from the channel
};
#endif