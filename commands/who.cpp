#include "../server.hpp"

// final version
void Server::_handle_who(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing WHO command" << std::endl;

    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        std::cout << "Client is not registered" << std::endl;
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    if (credentials.empty())
    {
        std::cout << "Not enough parameters for WHO command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " WHO :Not enough parameters\r\n");
        return;
    }

    std::string mask = credentials[0];
    std::map<std::string, Channel>::iterator channel_it = _channelsByName.find(mask);
    if (channel_it == _channelsByName.end())
    {
        std::cout << "Channel does not exist: " << mask << std::endl;
        user->queueResponseMessage("403 " + user->_obtainNickname() + " " + mask + " :No such channel\r\n");
        return;
    }

    Channel& channel = channel_it->second;

    if (!channel.hasMember(user))
    {
        std::cout << "Client is not in channel: " << mask << std::endl;
        user->queueResponseMessage("442 " + user->_obtainNickname() + " " + mask + " :You're not on that channel\r\n");
        return;
    }

    const std::vector<Client*>& channelMembers = channel.getMembers();
    for (std::vector<Client*>::const_iterator it = channelMembers.begin(); it != channelMembers.end(); ++it)
    {
        Client* memberClient = *it;
        std::string memberDetails = "352 " + user->_obtainNickname() + " " + mask + " " + memberClient->_obtainUsername() + " ";

        // Use a placeholder for the hostname, or just omit it
        memberDetails += "* ";

        memberDetails += _serverName + " " + memberClient->_obtainNickname() + " ";

        // Check if the user is an operator
        if (channel.isModerator(memberClient->_obtainNickname()))
        {
            memberDetails += "@";
        }

        memberDetails += " :0 " + memberClient->_obtainRealname() + "\r\n";
        user->queueResponseMessage(memberDetails);
    }

    user->queueResponseMessage("315 " + user->_obtainNickname() + " " + mask + " :End of /WHO list.\r\n");

    std::cout << "Sent WHO list for channel: " << mask << std::endl;
}
