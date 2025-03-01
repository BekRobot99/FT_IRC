#include "../server.hpp"

// updated to handle more edge cases
void Server::_handle_kick(Client* user, const std::vector<std::string>& credentials)
{


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
    std::string targetUserNickname  = credentials[1];
    std::string kickReason  = "No reason specified";

    if (credentials.size() >= 3)
		kickReason  = credentials[2];
	// Check if the client is an operator of the channel
	if (!_channelsByName.count(channelName))
	{
		user->queueResponseMessage("403 * " + channelName + " :No such channel\r\n");
		return;
	}
	Channel& channel = _channelsByName[channelName];

	if (!channel.isModerator(user->_obtainNickname()))
	{
		user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// Check if the target user is in the channel
	Client* targetClient  = _locateClientByNickname(targetUserNickname );
	if (targetClient  == NULL || !channel.hasMember(targetClient ))
	{
		user->queueResponseMessage("441 " + user->_obtainNickname() + " " + targetUserNickname  + " " + channelName + " :They aren't on that channel\r\n");
		return;
	}

	// Notify all users joined in the channel user
	std::string kickNotification  = ":" + user->_obtainNickname() + " KICK " + channelName + " " + targetUserNickname  + " :" + kickReason  + "\r\n";
	_distribute_msg_to_channel_members(targetClient , &channel, kickNotification , true);

	// Remove the target user from the channel
	channel.removeMember(targetClient );
}
