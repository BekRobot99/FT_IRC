#include "../server.hpp"

void Server::_handle_ping(Client* user, std::vector<std::string> credentials) {
    // Check if the origin parameter is provided
    if (credentials.empty())
    {
        user->queueResponseMessage("409 * :No origin specified\r\n");
        return;
    }

    // Construct the PONG response
    std::string origin = credentials[0];
    std::string pongMessage = ":" + _serverName + " PONG " + origin + "\r\n";

    // Send the PONG response
    user->queueResponseMessage(pongMessage);
}
