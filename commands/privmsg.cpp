#include "../server.hpp"

void Server::_handle_privmsg(Client* user, std::vector<std::string> credentials) {
    // Check if the client is in the correct state to send PRIVMSG
    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    // Check if all required parameters are provided
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
                continue;
            }

            Channel& channel = channelIt->second;
            if (!channel.hasMember(user))
            {
                user->queueResponseMessage("404 " + user->_obtainNickname() + " " + targetName + " :Cannot send to channel\r\n");
                continue;
            }

            // Relay the message to all clients in the channel
            std::string msgToSend = ":" + user->_obtainNickname() + " PRIVMSG " + targetName + " :" + message + "\r\n";
            _distributeMessageToChannelMembers(user, &channel, msgToSend, false);
        }
        else
        {
            // Target is a user
            Client* targetClient = _locateClientByNickname(targetName);
            if (!targetClient)
            {
                user->queueResponseMessage("401 " + user->_obtainNickname() + " " + targetName + " :No such nick\r\n");
                continue;
            }

            std::string msgToSend = ":" + user->_obtainNickname() + " PRIVMSG " + targetName + " :" + message + "\r\n";
            targetClient->queueResponseMessage(msgToSend);
        }
    }
}
