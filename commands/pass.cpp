#include "../server.hpp"

// updated
void Server::_handle_pass(Client* user, std::vector<std::string> credentials)
{
    std::cout << "Executing PASS command" << std::endl;
    if (user->getRegistrationStatus() != STATUS_PASS)
    {
        std::cout << "Client is already registered" << std::endl;
        user->queueResponseMessage("462 * :You may not reregister\r\n");
        return;
    }

    if (credentials.size() < 1)
    {
        std::cout << "Not enough parameters for PASS command" << std::endl;
        user->queueResponseMessage("461 " + user->_obtainNickname() + " PASS :Not enough parameters\r\n");
        return;
    }

    if (credentials[0] != _serverPassword)
    {
        std::cout << "Incorrect password" << std::endl;
        user->queueResponseMessage("464 * :Password incorrect\r\n");
        return;
    }

    std::cout << "Password accepted" << std::endl;
    user->updateRegistrationStatus();
}
