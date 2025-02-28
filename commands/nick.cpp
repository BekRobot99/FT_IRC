#include "../server.hpp"

// updated to handle more edge cases
void Server::_handle_nick(Client* user, std::vector<std::string> credentials)
{
    std::cout << "Executing NICK command" << std::endl;

    if (user->getRegistrationStatus() == STATUS_PASS)
    {
        std::cout << "Client has not registered yet" << std::endl;
        user->queueResponseMessage("451 * :You have not registered\r\n");
        return;
    }

    if (credentials.size() == 0)
    {
        std::cout << "No nickname provided" << std::endl;
        user->queueResponseMessage("431 * :No nickname given\r\n");
        return;
    }

    std::string nickname = credentials[0];
    if (!_checkNicknameValid(nickname))
    {
        std::cout << "Invalid nickname: " << nickname << std::endl;
        user->queueResponseMessage("432 * " + nickname + " :Erroneous nickname\r\n");
        return;
    }

    if (_isUsernameTaken(nickname))
    {
        std::cout << "Nickname already in use: " << nickname << std::endl;
        user->queueResponseMessage("433 * " + nickname + " :Nickname is already in use\r\n");
        return;
    }

    // Update the client's nickname
    std::string previousNickname = user->_obtainNickname();
    user->updateUsername(nickname);

    if (previousNickname.size() != 0)
    {
        std::vector<std::string>::iterator it = std::find(_registeredUsernames.begin(), _registeredUsernames.end(), previousNickname);
        if (it != _registeredUsernames.end())
        {
            _registeredUsernames.erase(it);
        }
    }

    _registeredUsernames.push_back(nickname);

    // If the client was already registered, send a notification to other users in the same channels
    if (user->getRegistrationStatus() == STATUS_REGISTERED)
    {
        std::string nick_change_msg = ":" + previousNickname + " NICK " + nickname + "\r\n";
        _distributeMessageToChannelMembers(nick_change_msg);
        std::cout << "Broadcasted nickname change: " << previousNickname << " -> " << nickname << std::endl;
        return;
    }

    // If status is NICK, proceed to USER
    if (user->getRegistrationStatus() == STATUS_NICK)
    {
        std::cout << "Proceeding to USER command" << std::endl;
        user->updateRegistrationStatus();
    }
}
