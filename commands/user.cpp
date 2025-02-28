#include "../server.hpp"

// updated to handle more edge cases
void Server::_handle_user(Client* user, std::vector<std::string> credentials)
{
    std::cout << "Executing USER command" << std::endl;

    if (user->getRegistrationStatus() != STATUS_USER)
    {
        std::cout << "Client is not in the correct state for USER command" << std::endl;
        user->queueResponseMessage("462 * :You may not reregister\r\n");
        return;
    }

    if (credentials.size() < 4)
    {
        std::cout << "Not enough parameters for USER command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " USER :Not enough parameters\r\n");
        return;
    }

    // std::string username = credentials[0];
    // std::string hostname = credentials[1];
    // std::string servername = credentials[2];
    // std::string realname = credentials[3];

    // Set the user's username and realname
    user->storeUsername(credentials[0]);
    user->storeRealname(credentials[3]);

    std::cout << "Username set to: " << credentials[0] << std::endl;
    std::cout << "Realname set to: " << credentials[3] << std::endl;

    _sendWelcomeMessage(user);
}
