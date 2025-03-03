#include "../server.hpp"

// final version
void Server::_handle_ping(Client* user, const std::vector<std::string>& credentials)
{
    std::cout << "Executing PING command" << std::endl;

    if (credentials.size() < 1)
    {
        std::cout << "No origin specified for PING command" << std::endl;
        user->queueResponseMessage("409 * :No origin specified\r\n");
        return;
    }

    std::string pongMessage = "PONG " + credentials[0] + "\r\n";
    user->queueResponseMessage(pongMessage);

    std::cout << "Responded to PING with PONG: " << credentials[0] << std::endl;
}
