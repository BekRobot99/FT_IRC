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
}
