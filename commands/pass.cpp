#include "../server.h"

void Server::_handle_pass(Client* user, std::vector<std::string> credentials)
{
    // Check if the client is in the correct state to send PASS
    if (user->getRegistrationStatus() != STATUS_PASS)
    {
        user->queueResponseMessage("462 * :You may not reregister\r\n");
        return;
    }

}
