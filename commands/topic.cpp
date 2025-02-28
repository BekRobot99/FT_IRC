#include "../server.hpp"

// updated to handle  more edge cases
void Server::_handle_topic(Client* user, const std::vector<std::string>& credentials)
{


	if (user->getRegistrationStatus() != STATUS_REGISTERED)
	{
		user->queueResponseMessage("451 * :You have not registered\r\n");
		return;
	}

    // Check if channel is given
    if (credentials.size() < 1)
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    std::string channelName = credentials[0];

    // Check if target_channel exists
    if (_channelsByName.find(channelName) == _channelsByName.end())
    {
        user->queueResponseMessage("403 * " + channelName + " :No such channel\r\n");
        return;
    }

    Channel& channel = _channelsByName[channelName];

    // Check if executing user is on channel
    if (!channel.hasMember(user))
    {
        user->queueResponseMessage("442 * " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // When credentials is one you want to get the topic
    if (credentials.size() == 1)
    {
        if (channel.obtainTopic() == std::string(""))
        {
            user->queueResponseMessage("331 " + channelName + " :No topic is set\r\n");
        }
        else
        {
            client->queueResponseMessage("332 " + channelName + " :" + channel.obtainTopic() + "\r\n");
        }
        return;
    }

    // Check if operator permissions are needed && if ther are check if the executing user has them
    if (channel.isTopicRestricted() && !channel.isModerator(user->_obtainNickname()))
    {
		user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
		return;
	}

	std::string newTopic = credentials[1];
	channel.updateDiscussionTopic(newTopic);
	_distribute_msg_to_channel_members(user, &channel, ":" + user->_obtainNickname() + " TOPIC " + channelName + " :" + target_channel.obtainTopic() + "\r\n", true);
}
