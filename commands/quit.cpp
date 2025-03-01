#include "../server.hpp"

// add message to the queue of the client
void Server::_handle_quit(Client* user, const std::vector<std::string>& credentials)
{
    std::string exitMessage = "Client Quit";
    if (credentials.size() >= 1)
    {
        exitMessage = credentials[0];
    }

    std::cout << "Client " << user->_obtainNickname() << " is quitting: " << exitMessage << std::endl;
    _handleClientDisconnection(user, exitMessage);
}
