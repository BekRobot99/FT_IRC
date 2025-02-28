#include "../server.hpp"

//updated to handle multiple channels + more edge cases
void Server::_handle_join(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing JOIN command" << std::endl;

    if (credentials[0].empty())
    {
        std::cout << "No channel specified" << std::endl;
        user->queueResponseMessage("403 * :No such channel\r\n");
        return;
    }

    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        std::cout << "Client is not registered" << std::endl;
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    if (credentials.empty())
    {
        std::cout << "Not enough parameters for JOIN command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }

    std::vector<std::string> channels = _tokenizeString(credentials[0], ',');
    std::vector<std::string> key;
    if (credentials.empty())
    {
        key = _tokenizeString(credentials[1], ',');
    }

    for (size_t i = 0; i < channels.size(); ++i)
    {
        const std::string& channelName = channels[i];
        std::string key = i < key.size() ? key[i] : "";

        if (_channelsByName.find(channelName) == _channelsByName.end())
        {
            // Create the channel if it doesn't exist
            _channelsByName[channelName] = Channel(channelName);
            _channelsByName[channelName].storePassword(key);
            // Grant operator status to the creator
            _channelsByName[channelName].assignModerator(user->_obtainNickname());
            std::cout << "Created new channel: " << channelName << std::endl;
        }

        Channel& channel = _channelsByName[channelName];

        if (channel.hasMember(user))
        {
            std::cout << "Client is already in channel: " << channelName << std::endl;
            return;
        }

        if (channel.isAtCapacity())
        {
            std::cout << "Channel is full: " << channelName << std::endl;
            user->queueResponseMessage("471 * " + channelName + " :Cannot join channel (+l)\r\n");
            return;
        }

        if (channel.isBlocked(user->_obtainNickname()))
        {
            std::cout << "Client is banned from channel: " << channelName << std::endl;
            user->queueResponseMessage("474 * " + channelName + " :Cannot join channel (+b)\r\n");
            return;
        }

        if (channel.hasRestrictedAccess() && !channel.isUserInvited(user->_obtainNickname()))
        {
            std::cout << "Channel is invite-only: " << channelName << std::endl;
            user->queueResponseMessage("473 * " + channelName + " :Cannot join channel (+i)\r\n");
            return;
        }

        if (channel.obtain_Password() != "" && key != "x" && key != channel.obtain_Password())
        {
            std::cout << "Incorrect channel password: " << channelName << std::endl;
            user->queueResponseMessage("475 * " + channelName + " :Cannot join channel (+k)\r\n");
            return;
        }

        channel.addMember(user);
        user->enterChannel(channelName, &channel);
        std::string joinMessage = ":" + user->_obtainNickname() + "!~" + user->_obtainUsername() + " JOIN " + channelName + "\r\n";
        _distribute_msg_to_channel_members(user, &channel, joinMessage, false);

        // Send the list of users in the channel to the client
        std::string namesList = channel.getMemberList();
        if (channel.obtainTopic() == "")
        {
            user->queueResponseMessage("331 " + user->_obtainNickname() + " " + channelName + " :No topic is set" + "\r\n");
        }
        else
        {
            user->queueResponseMessage("332 " + user->_obtainNickname() + " " + channelName + " :" + channel.obtainTopic() + "\r\n");
        }

        user->queueResponseMessage("353 " + user->_obtainNickname() + " = " + channelName + " :" + namesList + " \r\n");
        user->queueResponseMessage("366 " + user->_obtainNickname() + " " + channelName + " :End of /NAMES list\r\n");

        std::cout << "Client joined channel: " << channelName << std::endl;
    }
}
