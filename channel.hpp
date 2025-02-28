#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <string>
#include <set>
#include "client.hpp"

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
    void                            addInvitedUser(const std::string& user); // Add an invited user
    bool                            isUserInvited(const std::string& nickname) const; // Check if a nickname is invited
    void                            removeMember(Client* user); // Remove a client from the channel
    bool                            isAtCapacity() const; // Check if the channel is at capacity
    bool                            isBlocked(const std::string& nickname) const; // Check if a nickname is blocked
    bool                            hasRestrictedAccess() const; // Check if the channel has restricted access
    std::string                     obtain_Password() const; // Get the channel password
    std::string                     getMemberList(); // Get the channel topic
    std::string                     getDiscussionTopic() const; // Get the channel topic
    bool                            isTopicRestricted() const; // Check if the topic is restricted
    void                            configureInviteOnly(bool isInviteOnly); // Configure the channel to be invite-only
    void                            updateTopicRestriction (bool isTopicRestricted); // Update the topic restriction
    void                            removeModerator(const std::string& nickname); // Remove a moderator
    void                             setMemberLimit(unsigned short memberLmit); // Set the member limit
    short                           getTotalMembers() const; // Get the total number of members
    unsigned short                  getMemberLimit() const; // Get the member limit
    std::string                     getGroupName() const; // Get the channel name
    std::string                     generateModeFlags() const; // Generate the mode flags
};
#endif