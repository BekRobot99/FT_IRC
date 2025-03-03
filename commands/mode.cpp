#include "../server.hpp"

// final version
void Server::_handle_mode(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing MODE command" << std::endl;

    if (user->getRegistrationStatus() != STATUS_REGISTERED)
    {
        std::cout << "Client is not registered" << std::endl;
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    if (credentials.size() < 1)
    {
        std::cout << "Not enough parameters for MODE command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string clientTarget = credentials[0];
    if (clientTarget[0] == '#' || clientTarget[0] == '&')
    {
        // Channel mode
        _handle_channel_mode(user, credentials);
    }
    else
    {
        // User mode
        _handle_user_mode(user, credentials);
        user->queueResponseMessage("502 " + user->_obtainNickname() + " :Cannot change mode for other users\r\n");
    }
}

// updated
void Server::_handle_channel_mode(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing CHANNEL MODE command" << std::endl;

    if (credentials.size() < 1)
    {
        std::cout << "Not enough parameters for CHANNEL MODE command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string channelName = credentials[0];
    std::map<std::string, Channel>::iterator channelIt = _channelsByName.find(channelName);
    if (channelIt == _channelsByName.end())
    {
        std::cout << "Channel does not exist: " << channelName << std::endl;
        user->queueResponseMessage("403 " + user->_obtainNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel& channel = channelIt->second;

    if (!channel.hasMember(user))
    {
        std::cout << "Client is not in channel: " << channelName << std::endl;
        user->queueResponseMessage("442 " + user->_obtainNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    if (credentials.size() == 1)
    {
        // View the current modes
        std::string modestr = channel.generateModeFlags();
        user->queueResponseMessage("324 " + user->_obtainNickname() + " " + channelName + " " + modestr + "\r\n");
        std::cout << "Current modes for channel " << channelName << ": " << modestr << std::endl;
        return;
    }

    if (!channel.isModerator(user->_obtainNickname()))
    {
        std::cout << "Client is not an operator in channel: " << channelName << std::endl;
        user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    std::string modeString = credentials[1];
    bool isAddingMode = true;
    size_t paramIdx = 2;

    for (size_t i = 0; i < modeString.length(); ++i)
    {
        char modeCharacter = modeString[i];

        if (modeCharacter == '+' || modeCharacter == '-')
        {
            isAddingMode = (modeCharacter == '+');
            continue;
        }

        switch (modeCharacter)
        {
            case 'i':
                channel.configureInviteOnly(isAddingMode);
                _distribute_msg_to_channel_members(user, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " " + (isAddingMode ? "+i" : "-i") + "\r\n", true);
                std::cout << "Channel " << channelName << " is now " << (isAddingMode ? "invite-only" : "not invite-only") << std::endl;
                break;

            case 't':
                channel.updateTopicRestriction(isAddingMode);
                _distribute_msg_to_channel_members(user, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " " + (isAddingMode ? "+t" : "-t") + "\r\n", true);
                std::cout << "Channel " << channelName << " topic is now " << (isAddingMode ? "restricted to operators" : "not restricted") << std::endl;
                break;

            case 'k':
                if (isAddingMode && paramIdx < credentials.size())
                {
                    std::string password = credentials[paramIdx++];
                    channel.storePassword(password);
                    _distribute_msg_to_channel_members(user, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " +k " + password + "\r\n", true);
                    std::cout << "Channel " << channelName << " password set to: " << password << std::endl;
                }
                else if (!isAddingMode)
                {
                    channel.storePassword("");
                    _distribute_msg_to_channel_members(user, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " -k\r\n", true);
                    std::cout << "Channel " << channelName << " password removed" << std::endl;
                }
                break;

            case 'l':
                if (isAddingMode && paramIdx < credentials.size())
                {
                    int user_limit = std::atoi(credentials[paramIdx++].c_str());
                    channel.setMemberLimit(user_limit);
                    _distribute_msg_to_channel_members(user, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " +l " + std::to_string(user_limit) + "\r\n", true);
                    std::cout << "Channel " << channelName << " user limit set to: " << user_limit << std::endl;
                }
                else if (!isAddingMode)
                {
                    channel.setMemberLimit(0);
                    _distribute_msg_to_channel_members(user, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " -l\r\n", true);
                    std::cout << "Channel " << channelName << " user limit removed" << std::endl;
                }
                break;

            case 'o':
                if (paramIdx < credentials.size())
                {
                    std::string targetNickname = credentials[paramIdx++];
                    Client* target_client = _locateClientByNickname(targetNickname);
                    if (target_client && channel.hasMember(target_client))
                    {
                        if (isAddingMode)
                        {
                            channel.assignModerator(targetNickname);
                        }
                        else
                        {
                            channel.removeModerator(targetNickname);
                        }
                        _distribute_msg_to_channel_members(user, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " " + (isAddingMode ? "+o" : "-o") + " " + targetNickname + "\r\n", true);
                        std::cout << "Operator status for " << targetNickname << " in channel " << channelName << " is now " << (isAddingMode ? "added" : "removed") << std::endl;
                    }
                    else
                    {
                        user->queueResponseMessage("401 " + user->_obtainNickname() + " " + targetNickname + " :No such nick\r\n");
                    }
                }
                break;

            default:
            user->queueResponseMessage("472 " + user->_obtainNickname() + " " + std::string(1, modeCharacter) + " :is unknown mode char to me\r\n");
                break;
        }
    }
}


// final version
void Server::_handle_user_mode(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing USER MODE command" << std::endl;

    if (credentials.size() < 1)
    {
        std::cout << "Not enough parameters for USER MODE command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string targetNickname = credentials[0];
    if (targetNickname != user->_obtainNickname())
    {
        std::cout << "Client cannot change modes for other users" << std::endl;
        user->queueResponseMessage("502 " + user->_obtainNickname() + " :Cannot change mode for other users\r\n");
        return;
    }

    if (credentials.size() == 1)
    {
        // View the current modes
        std::string mode_str = "+i"; // Example: Only +i (invisible) is supported for users
        user->queueResponseMessage("221 " + user->_obtainNickname() + " " + mode_str + "\r\n");
        std::cout << "Current modes for user " << targetNickname << ": " << mode_str << std::endl;
        return;
    }

    std::string modeString = credentials[1];
    bool isAddingMode = true;

    for (size_t i = 0; i < modeString.length(); ++i)
    {
        char modeCharacter = modeString[i];

        if (modeCharacter == '+' || modeCharacter == '-')
        {
            isAddingMode = (modeCharacter == '+');
            continue;
        }

        switch (modeCharacter)
        {
            case 'i':
                // Toggle invisible mode
                user->queueResponseMessage("221 " + user->_obtainNickname() + " " + (isAddingMode ? "+i" : "-i") + "\r\n");
                std::cout << "User " << targetNickname << " is now " << (isAddingMode ? "invisible" : "visible") << std::endl;
                break;

            default:
            user->queueResponseMessage("472 " + user->_obtainNickname() + " " + std::string(1, modeCharacter) + " :is unknown mode char to me\r\n");
                break;
        }
    }
}
