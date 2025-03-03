#include "../server.hpp"

// final version
void Server::_handle_topic(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing TOPIC command" << std::endl;

    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        std::cout << "Client is not registered" << std::endl;
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    if (credentials.size() < 1)
    {
        std::cout << "Not enough parameters for TOPIC command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " TOPIC :Not enough parameters\r\n");
        return;
    }

    std::string channelName = credentials[0];
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

    if (credentials.size() == 1)
    {
        // View the topic
        if (channel.obtainTopic().empty())
        {
            std::cout << "No topic is set for channel: " << channelName << std::endl;
            user->queueResponseMessage("331 " + user->_obtainNickname() + " " + channelName + " :No topic is set\r\n");
        }
        else
        {
            std::cout << "Topic for channel " << channelName << ": " << channel.obtainTopic() << std::endl;
            user->queueResponseMessage("332 " + user->_obtainNickname() + " " + channelName + " :" + channel.obtainTopic() + "\r\n");
        }
    }
    else
    {
        // Set the topic
        if (channel.isTopicRestricted() && !channel.isModerator(user->_obtainNickname()))
        {
            std::cout << "Client is not an operator in channel: " << channelName << std::endl;
            user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
            return;
        }

        std::string newTopic = credentials[1];
        channel.updateDiscussionTopic(newTopic);
        std::string topic_msg = ":" + user->_obtainNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
        _distribute_msg_to_channel_members(user, &channel, topic_msg, true);

        std::cout << "Topic for channel " << channelName << " set to: " << newTopic << std::endl;
    }
}
