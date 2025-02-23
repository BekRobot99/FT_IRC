#include "../server.hpp"

void Server::_handle_join(Client* user, std::vector<std::string> credentials) {
    // Check if the client is in the correct state to send JOIN
    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    // Check if the channel parameter is provided
    if (credentials.empty())
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }

    std::string channelName = credentials[0];
    std::string key = credentials.size() > 1 ? credentials[1] : "";

    // Create the channel if it doesn't exist
    if (_channelsByName.find(channelName) == _channelsByName.end())
    {
        _channelsByName[channelName] = Channel(channelName);
        _channelsByName[channelName].storePassword(key);
        // Grant operator status to the creator
        _channelsByName[channelName].assignModerator(user->_obtainNickname());
    }

    Channel* channel = &_channelsByName[channelName];

    // Check if the client is already in the channel
    if (channel->hasMember(user))
    {
        return;
    }

    // Check if the channel is full
    if (channel->isAtCapacity())
    {
        user->queueResponseMessage("471 * " + channelName + " :Cannot join channel (+l)\r\n");
        return;
    }

    // Check if the client is banned from the channel
    if (channel->isBlocked(user->_obtainNickname()))
    {
        user->queueResponseMessage("474 * " + channelName + " :Cannot join channel (+b)\r\n");
        return;
    }

    // Check if the channel is invite-only and the client is not invited
    if (channel->hasRestrictedAccess() && !channel->isUserInvited(user->_obtainNickname()))
    {
        user->queueResponseMessage("473 * " + channelName + " :Cannot join channel (+i)\r\n");
        return;
    }

    // Check if the channel has a password and the provided key is incorrect
    if (channel->obtain_Password() != "" && key != "x" && key != channel->obtain_Password())
    {
        user->queueResponseMessage("475 * " + channelName + " :Cannot join channel (+k)\r\n");
        return;
    }
}
