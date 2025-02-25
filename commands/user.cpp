#include "../server.hpp"

void Server::_handle_user(Client* user, std::vector<std::string> credentials) {
    // Check if the client is in the correct state to send USER
    if (user->getRegistrationStatus() != STATUS_USER)
    {
        user->queueResponseMessage("462 * ERROR : You may not reregister\r\n");
        return;
    }

    // Check if all required parameters are provided
    if (credentials.size() < 4)
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + " USER :Not enough parameters\r\n");
        return;
    }

    user->storeUsername(credentials[0]); 
    user->storeRealname(credentials[3]);

    // Check if the client has both a nickname and a username
    if (!user->_obtainNickname().empty() && !user->_obtainUsername().empty())
    {
        // Update the client's registration status
        user->updateRegistrationStatus();

        // Send a welcome message to the client
        _sendWelcomeMessage(user);
    }
}
