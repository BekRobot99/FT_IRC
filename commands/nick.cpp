#include "../server.hpp"

void Server::_handle_nick(Client* user, std::vector<std::string> credentials) {
    // Check if the client is in the correct state to send NICK
    if (user->getRegistrationStatus() == STATUS_PASS)
    {
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    // Check if the nickname parameter is provided
    if (credentials.empty())
    {
        user->queueResponseMessage("431 * ERROR : No nickname given\r\n");
        return;
    }

    std::string nickname = credentials[0];

    // Validate the nickname
    if (!_checkNicknameValid(nickname))
    {
        user->queueResponseMessage("432 * " + nickname + " :Erroneous nickname\r\n");
        return;
    }

    // Check if the nickname is already in use
    if (_isUsernameTaken(nickname))
    {
        user->queueResponseMessage("433 * " + nickname + " :Nickname is already in use\r\n");
        return;
    }

    // Update the client's nickname
    std::string previousNickname = user->_obtainNickname();
    user->updateUsername(nickname);

    // Remove the old nickname from the registered usernames list
    if (!previousNickname.empty())
    {
        std::vector<std::string>::iterator it = std::find(_registeredUsernames.begin(), _registeredUsernames.end(), previousNickname);
        if (it != _registeredUsernames.end())
            _registeredUsernames.erase(it);
    }

    // Add the new nickname to the registered usernames list
    _registeredUsernames.push_back(nickname);

    // If the client was already registered, broadcast the nickname change
    if (user->getRegistrationStatus() == STATUS_REGISTERED)
    {
        std::string nick_change_msg = ":" + previousNickname + " NICK " + nickname + "\r\n";
        _notifyAllSubscribedChannels(nick_change_msg);
        return;
    }

    // If status is NICK, proceed to USER
    if (user->getRegistrationStatus() == STATUS_NICK)
        user->updateRegistrationStatus();
}
