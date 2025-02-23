#include "../server.hpp"

void Server::_handle_quit(Client* user, std::vector<std::string> credentials) {
    std::string exitMessage = ":" + user->_obtainNickname() + " QUIT :";
    if (!credentials.empty()) {
        exitMessage += credentials[0];
    }
    else {
        exitMessage += "Client disconnected";
    }
    // Disconnect the client
    _disconnectClient(user, exitMessage);
}