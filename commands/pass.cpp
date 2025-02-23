#include "../server.h"

void Server::_handle_pass(Client* user, std::vector<std::string> credentials)
{
    // Check if the client is in the correct state to send PASS
    if (user->getRegistrationStatus() != STATUS_PASS)
    {
        user->queueResponseMessage("462 * :You may not reregister\r\n");
        return;
    }

    // Check if the password parameter is provided
    if (credentials.empty())
    {
        user->queueResponseMessage("461 " + user->_obtainNickname() + "ERROR :Not enough parameters (PASS)\r\n");
        return;
    }

    // Check if the user is already registered
    if (user->is_registered())
    {
        user->queueResponseMessage("462 * :You may not reregister\r\n");
        return;
    }

    // Check if the password is correct
    if (credentials[0] != _serverPassword)
    {
        user->queueResponseMessage("464 * :Password incorrect\r\n");
        return;
    }

    // Store the password
    user->storePassword(credentials[0]);

    // Proceed to the next registration status
    user->updateRegistrationStatus();
}
