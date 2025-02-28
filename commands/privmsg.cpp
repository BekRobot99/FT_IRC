#include "../server.hpp"

// updated to handle multiple channels + more edge cases
void Server::_handle_privmsg(Client* user, const std::vector<std::string>& credentials)
{

	if (user->getRegistrationStatus() != STATUS_REGISTERED)
	{
		user->queueResponseMessage("451 * :You have not registered\r\n");
		return;
	}

	if (credentials.size() < 2)
	{
		user->queueResponseMessage("461 " + user->_obtainNickname() + " PRIVMSG :Not enough parameters\r\n");
		return;
	}	
	std::vector<std::string> targetNames = _tokenizeString(credentials[0], ',');
	eliminateDuplicateEntries(targetNames);
	std::string message = credentials[1];
	for (size_t i = 0; i < targetNames.size(); i++)
	{
		std::string targetName = targetNames[i];
		// Check if the target is a channel or a user
		if (targetName[0] == '#')
		{
			// Target is a channel
			std::map<std::string, Channel>::iterator channelIt = _channelsByName.find(targetName);
			if (channelIt == _channelsByName.end())
			{
				user->queueResponseMessage("403 " + user->_obtainNickname() + " " + targetName + " :No such channel\r\n");
				return;
			}	
			Channel& channel = channelIt->second;
			if (!channel.hasMember(user))
			{
				user->queueResponseMessage("404 " + user->_obtainNickname() + " " + targetName + " :Cannot send to channel\r\n");
				return;
			}


			// Relay the message to all clients in the channel
			const std::vector<Client*>& clientsInChannel = channel.getMembers();
			for (std::vector<Client*>::const_iterator it = clientsInChannel.begin(); it != clients_in_channel.end(); ++it)
			{
				Client* clientInChannel = *it;
				if (clientInChannel != user)
				{
					std::string msgToSend = ":" + user->_obtainNickname() + " PRIVMSG " + targetName + " :" + message + "\r\n";
					clientInChannel->queueResponseMessage(msgToSend);
				}
			}
		}
		else
		{
			Client* targetClient = _locateClientByNickname(targetName);
			if (targetClient == NULL)
			{
				client->queueResponseMessage("401 " + user->_obtainNickname() + " " + targetName + " :No such nick\r\n");
				return;
			}
			std::string msgToSend = ":" + user->_obtainNickname() + " PRIVMSG " + targetName + " :" + message + "\r\n";
			targetClient->queueResponseMessage(msgToSend);
		}
	}
}
