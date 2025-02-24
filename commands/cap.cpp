#include "../server.hpp"

void Server::_handle_cap(Client* user, const std::vector<std::string>& credentials) {
    if (credentials.empty()) {
        user->queueResponseMessage("421 * CAP :Not enough parameters\r\n");
        return;
    }

    std::string subcommand = credentials[0];
    if (subcommand == "LS") {
        user->queueResponseMessage("CAP * LS :multi-prefix sasl\r\n");
    } else if (subcommand == "REQ") {
        user->queueResponseMessage("CAP * ACK :multi-prefix sasl\r\n");
    } else if (subcommand == "END") {
        user->queueResponseMessage("CAP * END :multi-prefix sasl\r\n");
    } else {
        user->queueResponseMessage("421 * CAP :Unknown subcommand\r\n");
    }
}
