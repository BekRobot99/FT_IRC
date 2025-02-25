#include "../server.hpp"

void Server::_handle_who(Client* user, const std::vector<std::string>& credentials)
{
    if (credentials.empty())
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " WHO :Not enough parameters\r\n");
        return;
    }

    std::string mask = credentials[0];

    if (!_channelsByName.count(mask))
    {
        user->queueResponseMessage("403 * " + mask + " :No such channel\r\n");
        return;
    }
    
    Channel& channel = _channelsByName[mask];

    if (!channel.hasMember(user))
    {
        user->queueResponseMessage("442 " + user->_obtainNickname() + " " + mask + " :You're not on that channel\r\n");
        return;
    }

	const std::vector<Client*>& channelMembers = channel.getMembers();

	for (std::vector<Client*>::const_iterator it = channelMembers.begin(); it != channelMembers.end(); ++it)
	{
		Client* memberClient = *it;
        std::string memberDetails = "352 " + user->_obtainNickname() + " " + mask + " " + memberClient->_obtainUsername() + " ";

        memberDetails += "* ";

        memberDetails += _serverName + " " + memberClient->_obtainNickname() + " ";

        if (channel.isModerator(memberClient->_obtainNickname()))
        {
            memberDetails += "@";
        }
        
        memberDetails += " :0 " + memberClient->_obtainRealname() + "\r\n";
        user->queueResponseMessage(memberDetails);
    }

    user->queueResponseMessage("315 " + user->_obtainNickname() + " " + mask + " :End of /WHO list.\r\n");
}
