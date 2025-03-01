#include "../server.hpp"

// updated to handle  more edge cases
void Server::_handle_invite(Client* user, const std::vector<std::string>& credentials)
{

	if (user->getRegistrationStatus() != STATUS_REGISTERED)
	{
		user->queueResponseMessage("451 * :You have not registered\r\n");
		return;
	}

	// Check for enough parameters
	if (credentials.size() < 2)
	{
		user->queueResponseMessage("461 " + user->_obtainNickname() + " INVITE :Not enough parameters\r\n");
		return;
	}

	std::string targetNickname = credentials[0]; 
	std::string channelName = credentials[1];
	
	// Check if channel exists
	if (_channelsByName.find(channelName) == _channelsByName.end())
	{
		user->queueResponseMessage("403 * " + channelName + " :No such channel\r\n");
		return;
	}

	// Check if executing user is on channel
	Channel& channel = _channelsByName[channelName];
	if (!channel.hasMember(user))
	{
		user->queueResponseMessage("442 " + user->_obtainNickname() + " :You're not on that channel\r\n");
		return;
	}

	// Check if executing user is operator on channel
	if (!channel.isModerator(user->_obtainNickname()))
	{
		user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// Check if target_nickname exists
	Client* targetClient = _locateClientByNickname(targetNickname);
	if (targetClient == NULL)
	{
		user->queueResponseMessage("401 " + targetNickname + " :No such nick/channel\r\n");
		return;
	}

	// Check if target_nickname is already in the channel
	if (channel.hasMember(targetClient))
	{
		user->queueResponseMessage("443 " + targetNickname + " " + channelName + " :is already on channel\r\n");
		return;
	}

	// EXECUTE INVITE
	channel.addInvitedUser(targetNickname);

	targetClient->queueResponseMessage("341 " + targetNickname + " " + channelName + "\r\n");

}
