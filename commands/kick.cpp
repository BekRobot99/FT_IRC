#include "../server.hpp"

void Server::_handle_kick(Client* user, const std::vector<std::string>& credentials) {
    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    if (credentials.size() < 2)
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " KICK :Not enough parameters\r\n");
        return;
    }

    std::string channelName = credentials[0];
    std::string targetNickname = credentials[1];
    std::string reason = credentials.size() > 2 ? credentials[2] : "No reason specified";

    if (_channelsByName.find(channelName) == _channelsByName.end())
    {
        user->queueResponseMessage("403 * " + channelName + " :No such channel\r\n");
        return;
    }

    Channel* channel = &_channelsByName[channelName];

    if (!channel->isModerator(user->_obtainNickname()))
    {
        user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    Client* targetClient = _locateClientByNickname(targetNickname);
    if (!targetClient || !channel->hasMember(targetClient))
    {
        user->queueResponseMessage("441 " + user->_obtainNickname() + " " + targetNickname + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }

    std::string kickMessage = ":" + user->_obtainNickname() + " KICK " + channelName + " " + targetNickname + " :" + reason + "\r\n";
    _distributeMessageToChannelMembers(user, channel, kickMessage, true);

    channel->removeMember(targetClient);
    targetClient->exitChannel(channelName);
}