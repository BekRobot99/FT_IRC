#include "../server.hpp"

void Server::_handle_invite(Client* user, const std::vector<std::string>& credentials) {
    // Check if the client is in the correct state to send INVITE
    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    // Check if all required parameters are provided
    if (credentials.size() < 2)
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    std::string targetNickname = credentials[0];
    std::string channelName = credentials[1];

    // Check if the channel exists
    if (_channelsByName.find(channelName) == _channelsByName.end())
    {
        user->queueResponseMessage("403 * " + channelName + " :No such channel\r\n");
        return;
    }

    Channel* channel = &_channelsByName[channelName];

    // Check if the client is in the channel
    if (!channel->hasMember(user))
    {
        user->queueResponseMessage("442 " + user->_obtainNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // Check if the client is an operator on the channel
    if (!channel->isModerator(user->_obtainNickname()))
    {
        user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Check if the target user exists
    Client* targetClient = _locateClientByNickname(targetNickname);
    if (!targetClient)
    {
        user->queueResponseMessage("401 " + targetNickname + " :No such nick/channel\r\n");
        return;
    }

    // Check if the target user is already in the channel
    if (channel->hasMember(targetClient))
    {
        user->queueResponseMessage("443 " + targetNickname + " " + channelName + " :is already on channel\r\n");
        return;
    }

    // Add the target user to the invite list
    channel->addInvitedUser(targetClient);

    // Send the invite confirmation to the target user
    std::string inviteMessage = "341 " + targetNickname + " " + channelName + "\r\n";
    targetClient->queueResponseMessage(inviteMessage);

    // Send the invite notification to the inviting user
    std::string notificationMessage = ":" + user->_obtainNickname() + " INVITE " + targetNickname + " " + channelName + "\r\n";
    user->queueResponseMessage(notificationMessage);
}
