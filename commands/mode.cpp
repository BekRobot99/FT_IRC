#include "../server.hpp"

// updated
void Server::_handle_mode(Client* user, const std::vector<std::string>& credentials)
{
	if (user->getRegistrationStatus() != STATUS_REGISTERED)
	{
		user->queueResponseMessage("451 * :You have not registered\r\n");
		return;
	}

	if (credentials.size() < 1)
	{
		user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
		return;
	}
	std::string target = credentials[0];
	if (target[0] == '#' || target[0] == '&') // Channel mode
	{
		_handle_channel_mode(user, credentials);
	}
	else // User mode
	{
		_handle_user_mode(user, credentials);
		user->queueResponseMessage("502 " + user->_obtainNickname() + " :Cannot change mode for other users\r\n");
	}
}

// updated
void Server::_handle_channel_mode(Client* user, const std::vector<std::string>& credentials)
{
	if (user->getRegistrationStatus() != STATUS_REGISTERED)
	{
		user->queueResponseMessage("451 * :You have not registered\r\n");
		return;
	}


	std::string channelName = credentials[0];

	// Check if the channel exists
	std::map<std::string, Channel>::iterator channelIt = _channelsByName.find(channelName);
	if (channelIt == _channelsByName.end())
	{
		user->queueResponseMessage("403 " + user->_obtainNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel& channel = channelIt->second;

	// Check if the client is an operator of the channel
	if (credentials.size() == 1)
	{
		std::string modestr = channel.generateModeFlags(); 
		user->queueResponseMessage("324 " + user->_obtainNickname() + " " + channelName + " " + modestr + "\r\n");
		return;
	}

	if (credentials.size() == 2 && credentials[1] == "b")
	{
    	user->queueResponseMessage("368 " + user->_obtainNickname() + " " + channelName + " :End of Channel Ban List\r\n");
		return ;
	}
	if (!channel.isModerator(user->_obtainNickname()))
	{
		user->queueResponseMessage("482 " + channelName + " :You're not channel operator\r\n");
		return;
	}

	std::string modeString = credentials[1];

	size_t paramIdx = 1;

	while (paramIdx < credentials.size()) {
		std::string modeString = credentials[paramIdx++];
		bool set_mode = true;

		for (size_t i = 0; i < modeString.length(); ++i)
		{
			char mode_char = modeString[i];

			if (mode_char == '+' || mode_char == '-')
			{
				set_mode = (mode_char == '+');
			}
			else
			{
				switch (mode_char)
				{
					case 'i':
						channel.configureInviteOnly(set_mode);
						_distribute_msg_to_channel_members(NULL, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " " + (set_mode ? "+i" : "-i") + "\r\n", true);
						break;
					case 't':
						channel.updateTopicRestriction(set_mode);
						_distribute_msg_to_channel_members(NULL, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " " + (set_mode ? "+t" : "-t") + "\r\n", true);
						break;
					case 'k':
						if (paramIdx < credentials.size())
						{
							channel.storePassword(set_mode ? credentials[paramIdx++] : "");
						}
						else
						{
							user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
							return;
						}
						_distribute_msg_to_channel_members(NULL, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " " + (set_mode ? "+k" : "-k") + " " + (set_mode ? channel.obtain_Password() : "") + "\r\n", true);
						paramIdx++;
                        break;
                    case 'o':
                		if (paramIdx < credentials.size())
                		{
                		    std::string targetNickname = credentials[paramIdx++];
							Client* targetClient = _locateClientByNickname(targetNickname);
							if (targetClient)
							{
								if (set_mode)
									channel.assignModerator(targetNickname);
								else
									channel.removeModerator(targetNickname);
								_distribute_msg_to_channel_members(targetClient, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " " + (set_mode ? "+o" : "-o") + " " + targetNickname + "\r\n", true);
							}
							else
							{
								user->queueResponseMessage("401 " + user->_obtainNickname() + " " + targetNickname + " :No such nick\r\n");
							}
						}
						else
						{
							user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
                		}
                		break;
                    case 'l':
                        if (set_mode && paramIdx < credentials.size())
                        {
                            channel.setMemberLimit(atoi(credentials[paramIdx++].c_str()));
							std::stringstream ss;
							ss << channel.getMemberLimit();
							std::string user_limit_string = ss.str();
							_distribute_msg_to_channel_members(NULL, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " +l " + user_limit_string + "\r\n", true);
						}
						else if (!set_mode)
						{
							channel.setMemberLimit(0);
							_distribute_msg_to_channel_members(NULL, &channel, ":" + user->_obtainNickname() + " MODE " + channelName + " -l\r\n", true);
						}
						else
						{
							user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
							return;
						}
						break;
					default:
							user->queueResponseMessage("472 " + user->_obtainNickname() + " " + std::string(1, mode_char) + " :is unknown mode char to me\r\n");
						break;
				}
			}
		}
}}

void Server::_handle_user_mode(Client* user, const std::vector<std::string>& credentials)
{
    if (credentials.size() < 2)
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string target_nickname = credentials[0];
    std::string modeString = credentials[1];

    Client* target_client = _locateClientByNickname(target_nickname);
    if (target_client == NULL)
    {
        user->queueResponseMessage("401 " + user->_obtainNickname() + " " + target_nickname + " :No such nick\r\n");
        return;
    }

    if (modeString.length() > 1)
    {
        user->queueResponseMessage("472 " + user->_obtainNickname() + " " + modeString + " :is unknown mode char to me\r\n");
        return;
    }

    char mode_char = modeString[0];
    if (mode_char != '+' && mode_char != '-')
    {
        user->queueResponseMessage("472 " + user->_obtainNickname() + " " + modeString + " :is unknown mode char to me\r\n");
        return;
    }

    bool set_mode = (mode_char == '+');

    if (set_mode)
    {
        user->queueResponseMessage("502 " + user->_obtainNickname() + " :Cannot change mode for other users\r\n");
    }
    else
    {
        user->queueResponseMessage("502 " + user->_obtainNickname() + " :Cannot change mode for other users\r\n");
    }
}
