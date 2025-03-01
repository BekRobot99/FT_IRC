#include "../server.hpp"

// fixed
void Server::_handle_cap(Client* user, const std::vector<std::string>& credentials)
{
    if (credentials.size() > 0)
    {
        if (credentials[0] == "LS")
        {
            user->queueResponseMessage("CAP * LS :\r\n");
        }
        else if (credentials[0] == "END")
        {
        }
        else
        {
            user->queueResponseMessage("421 * CAP :Unknown command\r\n");
        }
    }
    else
    {
        user->queueResponseMessage("421 * CAP :Unknown command\r\n");
    }
}
