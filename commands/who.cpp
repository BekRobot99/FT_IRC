#include "../server.hpp"

// fix errors
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

    // Check if the client is in the channel
    if (!channel.hasMember(user))
    {
        user->queueResponseMessage("442 " + user->_obtainNickname() + " " + mask + " :You're not on that channel\r\n");
        return;
    }

    // Send the list of users in the channel to the requesting client
	const std::vector<Client*>& channelMembers = channel.getMembers();

	// Iterate through the clients and send the message to each of them
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

    // Send the end of the WHO list message
    user->queueResponseMessage("315 " + user->_obtainNickname() + " " + mask + " :End of /WHO list.\r\n");
}
