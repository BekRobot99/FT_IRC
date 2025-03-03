#include "../server.hpp"

// final version
void Server::_handle_kick(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing KICK command" << std::endl;

    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        std::cout << "Client is not registered" << std::endl;
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    if (credentials.size() < 2)
    {
        std::cout << "Not enough parameters for KICK command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " KICK :Not enough parameters\r\n");
        return;
    }

    std::string channelName = credentials[0];
    std::string targetUserNickname = credentials[1];
    std::string kickReason = credentials.size() > 2 ? credentials[2] : "No reason specified";

    std::map<std::string, Channel>::iterator channelIt = _channelsByName.find(channelName);
    if (channelIt == _channelsByName.end())
    {
        std::cout << "Channel does not exist: " << channelName << std::endl;
        user->queueResponseMessage("403 " + user->_obtainNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel& channel = channelIt->second;

    if (!channel.isModerator(user->_obtainNickname()))
    {
        std::cout << "Client is not an operator in channel: " << channelName << std::endl;
        user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    Client* targetClient = _locateClientByNickname(targetUserNickname);
    if (targetClient == nullptr || !channel.hasMember(targetClient))
    {
        std::cout << "Target client is not in channel: " << channelName << std::endl;
        user->queueResponseMessage("441 " + user->_obtainNickname() + " " + targetUserNickname + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }

    std::string kickNotification = ":" + user->_obtainNickname() + " KICK " + channelName + " " + targetUserNickname + " :" + kickReason + "\r\n";
    _distribute_msg_to_channel_members(targetClient, &channel, kickNotification, true);

    channel.removeMember(targetClient);

    std::cout << "Kicked " << targetUserNickname << " from " << channelName << " for reason: " << kickReason << std::endl;
}
