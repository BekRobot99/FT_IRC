#include "../server.hpp"

void Server::_handle_topic(Client* user, const std::vector<std::string>& credentials) {
    // Check if the client is in the correct state to send TOPIC
    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    // Check if the channel parameter is provided
    if (credentials.empty())
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " TOPIC :Not enough parameters\r\n");
        return;
    }

    std::string channelName = credentials[0];

    // Check if the channel exists
    if (_channelsByName.find(channelName) == _channelsByName.end())
    {
        user->queueResponseMessage("403 * " + channelName + " :No such channel\r\n");
        return;
    }

    Channel* channel = &_channelsByName[channelName];

    // Check if the client is in the channel
    if (!channel->hasMember(user))
    {
        user->queueResponseMessage("442 * " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // When credentials.size() == 1, get the topic
    if (credentials.size() == 1)
    {
        std::string topic = channel->getDiscussionTopic();
        if (topic.empty())
        {
            user->queueResponseMessage("331 " + user->_obtainNickname() + " " + channelName + " :No topic is set\r\n");
        }
        else
        {
            user->queueResponseMessage("332 " + user->_obtainNickname() + " " + channelName + " :" + topic + "\r\n");
        }
        return;
    }

    // Check if operator permissions are needed and if the client has them
    if (channel->isTopicRestricted() && !channel->isModerator(user->_obtainNickname()))
    {
        user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Set the new topic
    std::string newTopic = credentials[1];
    channel->updateDiscussionTopic(newTopic);

    // Broadcast the new topic to all channel members
    std::string topicMessage = ":" + user->_obtainNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    _distributeMessageToChannelMembers(user, channel, topicMessage, true);
}
