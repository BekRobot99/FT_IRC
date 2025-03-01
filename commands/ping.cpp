#include "../server.hpp"

void	Server::_handle_ping(Client* user, std::vector<std::string> credentials)
{
	if (credentials.size() < 1)
	{
		user->queueResponseMessage("409 * :No origin specified\r\n");
		return;
	}

	std::string origin = credentials[0];
	std::string pongMessage = "PONG " + origin + "\r\n";
	// Send Pong
	user->queueResponseMessage(pongMessage);
}
