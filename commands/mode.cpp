#include "../server.cpp"

void Server::_handle_mode(Client* user, const std::vector<std::string>& credentials) {
    if (credentials.size() < 2) {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string target = credentials[0];

    if (target[0] == '#') {
        _handle_channel_mode(user, credentials);
    } else {
        _handle_user_mode(user, credentials);
        user->queueResponseMessage("502 " + user->_obtainNickname() + " :Cannot change mode for other users\r\n");
    }
}

void Server::_handle_channel_mode(Client* user, const std::vector<std::string>& credentials) {
    if (credentials.size() < 2) {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string channelName = credentials[0];
    std::string modeString = credentials[1];

    // Check if the channel exists
    std::map<std::string, Channel>::iterator it = _channelsByName.find(channelName);
    if (it == _channelsByName.end()) {
        user->queueResponseMessage("403 " + user->_obtainNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel& channel = it->second;

    // Check if the client is an operator of the channel
    if (!channel.isModerator(user->_obtainNickname())) {
        user->queueResponseMessage("482 " + channelName + " :You're not a channel operator\r\n");
        return;
    }

    size_t paramIdx = 2;
    bool setMode = true;

    for (size_t i = 0; i < modeString.length(); ++i) {
        char modeChar = modeString[i];

        if (modeChar == '+' || modeChar == '-') {
            setMode = (modeChar == '+');
            continue;
        }

        switch (modeChar) {
            case 'i':  // Invite-only mode
                channel.configureInviteOnly(setMode);
                break;

            case 't':  // Topic restriction mode
                channel.updateTopicRestriction(setMode);
                break;

            case 'k':  // Channel password mode
                if (setMode && paramIdx < credentials.size()) {
                    channel.storePassword(credentials[paramIdx++]);
                } else {
                    channel.storePassword("");
                }
                break;

            case 'o':  // Operator mode
                if (paramIdx < credentials.size()) {
                    std::string targetNickname = credentials[paramIdx++];
                    Client* targetClient = _locateClientByNickname(targetNickname);

                    if (!targetClient || !channel.hasMember(targetNickname)) {
                        user->queueResponseMessage("401 " + user->_obtainNickname() + " " + targetNickname + " :No such nick\r\n");
                        return;
                    }

                    if (setMode) {
                        channel.assignModerator(targetNickname);
                    } else {
                        channel.removeModerator(targetNickname);
                    }
                } else {
                    user->queueResponseMessage("461 " + user->_obtainNickname() + " MODE :Not enough parameters\r\n");
                    return;
                }
                break;
        }
    }
}
