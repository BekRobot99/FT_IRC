#include "../server.hpp"

// final version
void Server::_handle_invite(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing INVITE command" << std::endl;

    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        std::cout << "Client is not registered" << std::endl;
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    if (credentials.size() < 2)
    {
        std::cout << "Not enough parameters for INVITE command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    std::string targetNickname = credentials[0];
    std::string channelName = credentials[1];

    std::map<std::string, Channel>::iterator channel_it = _channelsByName.find(channelName);
    if (channel_it == _channelsByName.end())
    {
        std::cout << "Channel does not exist: " << channelName << std::endl;
        user->queueResponseMessage("403 " + user->_obtainNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel& channel = channel_it->second;

    if (!channel.hasMember(user))
    {
        std::cout << "Client is not in channel: " << channelName << std::endl;
        user->queueResponseMessage("442 " + user->_obtainNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    if (!channel.isModerator(user->_obtainNickname()))
    {
        std::cout << "Client is not an operator in channel: " << channelName << std::endl;
        user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    Client* targetClient = _locateClientByNickname(targetNickname);
    if (targetClient == nullptr)
    {
        std::cout << "Target client does not exist: " << targetNickname << std::endl;
        user->queueResponseMessage("401 " + user->_obtainNickname() + " " + targetNickname + " :No such nick\r\n");
        return;
    }

    if (channel.hasMember(targetClient))
    {
        std::cout << "Target client is already in channel: " << channelName << std::endl;
        user->queueResponseMessage("443 " + user->_obtainNickname() + " " + targetNickname + " " + channelName + " :is already on channel\r\n");
        return;
    }

    channel.addInvitedUser(targetNickname);
    targetClient->queueResponseMessage("341 " + user->_obtainNickname() + " " + targetNickname + " " + channelName + "\r\n");

    std::cout << "Invited " << targetNickname << " to " << channelName << std::endl;
}
