#include "server.hpp"
#include <map>
#include <vector>

// updated to avoid memory leaks
Server::Server(int port, std::string password, std::string server_name) : _serverPort(port), _serverPassword(password), _serverName(server_name), _numPollDescriptors(0), _listenerSocket(-1)
{
	time_t now = time(0);
	tm* creation_tm = localtime(&now);

	char buffer[20];
	strftime(buffer, sizeof(buffer), "%d-%b-%Y", creation_tm); // or "%b %d %Y" for an alternative format

	_serverCeationTime = std::string(buffer);
};

// updated to avoid memory leaks
Server::~Server()
{
	for (unsigned short it = 0; it < _numPollDescriptors; it++)
		close(_DescriptorsPoll[it].fd);
	close(_listenerSocket);
	std::cout << "END OF PROGRAM" << std::endl;
};

// Set up the server socket
void Server::_setup_server_socket() {
    _listenerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenerSocket < 0) {
        std::cerr << "Error: Could not create socket\n";
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(_listenerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error: Could not set socket options\n";
        exit(EXIT_FAILURE);
    }

    // Bind the socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_serverPort);

    if (bind(_listenerSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not bind socket\n";
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(_listenerSocket, SOMAXCONN) < 0) {
        std::cerr << "Error: Could not listen on socket\n";
        exit(EXIT_FAILURE);
    }

    // Add the listener socket to the pollfd vector
    struct pollfd listenerPollFd;
    listenerPollFd.fd = _listenerSocket;
    listenerPollFd.events = POLLIN;
    _DescriptorsPoll.push_back(listenerPollFd);
    _numPollDescriptors++;

    std::cout << "Server is listening on port " << _serverPort << "\n";
}

void Server::startRun()
{
    struct sockaddr_in listenerAddress;
    memset(&listenerAddress, 0, sizeof(listenerAddress));

    // Set attributes of the socket
    listenerAddress.sin_port = htons(_serverPort);
    listenerAddress.sin_family = AF_INET;
    listenerAddress.sin_addr.s_addr = INADDR_ANY;

    // Create a socket to receive incoming connections (LISTENER SOCKET)
    _listenerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenerSocket < 0)
        throw std::runtime_error("Failed To Open Socket");

    int socketOption = 1;
    // Set SO_REUSEADDR option to allow reuse of local addresses
    if (setsockopt(_listenerSocket, SOL_SOCKET, SO_REUSEADDR, &socketOption, sizeof(socketOption)))
        throw std::runtime_error("Failed To Set Socket To Be Reusable");

    // Set O_NONBLOCK flag to enable nonblocking I/O
    if (fcntl(_listenerSocket, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("Failed To Set Socket To Non-Blocking");

    // Set SO_NOSIGPIPE option to prevent SIGPIPE signals on write errors
    if (setsockopt(_listenerSocket, SOL_SOCKET, SO_NOSIGPIPE, &socketOption, sizeof(socketOption)))
        throw std::runtime_error("Failed To Set Socket To Not SIGPIPE");

    // Bind the listening socket to the port configured
    if (bind(_listenerSocket, reinterpret_cast<struct sockaddr*>(&listenerAddress), sizeof(listenerAddress)) < 0)
        throw std::runtime_error("Failed To Bind Socket To Port");

    // Sets the fd into passive mode and allows a maximum of 128 requests to queue up
    if (listen(_listenerSocket, 128) < 0)
        throw std::runtime_error("Failed To Mark Socket As Passive");

    // First fd in the array is the listener
    struct pollfd listenerDescriptor;
    std::memset(&listenerDescriptor, 0, sizeof(listenerDescriptor));
    listenerDescriptor.fd = _listenerSocket;
    listenerDescriptor.events = POLLIN;
    _DescriptorsPoll.push_back(listenerDescriptor);
    _numPollDescriptors++;

    _handleEvents();
}

// updated to avoid memory leaks
void Server::_handleEvents()
{
    // 512 is max in IRC, including \r\n
    while (true)
    {
        int updatedSocketCount = poll(&_DescriptorsPoll[0], _numPollDescriptors, 0);

        if (updatedSocketCount < 0)
            throw std::runtime_error("Failed To Poll Socket/s");

        for (int index = 0; index < _numPollDescriptors; index++)
        {
            if (_DescriptorsPoll[index].revents == 0)
                continue;

            // Error on socket -> remove from pollfds
            if ((_DescriptorsPoll[index].revents & POLLERR) == POLLERR)
            {
                _deleteClient(&_clientsBySocket[_DescriptorsPoll[index].fd]);
                break;
            }

            if ((_DescriptorsPoll[index].revents & POLLHUP) == POLLHUP)
            {
                _deleteClient(&_clientsBySocket[_DescriptorsPoll[index].fd]);
                break;
            }

            // New client is connecting
            if (_DescriptorsPoll[index].fd == _listenerSocket && (_DescriptorsPoll[index].revents & POLLIN) == POLLIN)
                _acceptClientConnection();
            else
            {
                if (_DescriptorsPoll[index].revents & POLLIN)
                    _processClientData(_DescriptorsPoll[index].fd);
            }
        }

        for (int index = 0; index < _numPollDescriptors; index++)
        {
            std::map<int, Client>::iterator clientIterator = _clientsBySocket.find(_DescriptorsPoll[index].fd);
            if (clientIterator == _clientsBySocket.end())
                continue;

            Client& activeClient = clientIterator->second;

            if (!activeClient.hasPendingResponse())
                continue;
                
            if (activeClient.transmitResponse())
            {
                _deleteClient(&activeClient);
            }
        }
    }
}

// updated to fix error handling
void Server::_acceptClientConnection()
{
    std::cout << "ACCEPT NEW CONNECTION" << std::endl;
    while (true)
    {
        int clientSocket = accept(_DescriptorsPoll[0].fd, NULL, NULL);
        if (clientSocket < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "Failed To Open Socket -> Client Could Not Connect" << std::endl;
                return;
            }
            else
                return ;
        }

        std::cout << "Accepted Client with fd: " << clientSocket << std::endl;

        // Set O_NONBLOCK flag to enable nonblocking I/O
        if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
        {
            std::cerr << "Failed To Set Socket To Non-Blocking -> Client Could Not Connect" << std::endl;
            return;
        }
        int socketOptionValue  = 1;
        // Set SO_NOSIGPIPE option to prevent SIGPIPE signals on write errors
        if (setsockopt(clientSocket, SOL_SOCKET, SO_NOSIGPIPE, &socketOptionValue , sizeof(socketOptionValue )))
        {
            std::cerr << "Failed To Set Socket To Not SIGPIPE -> Client Could Not Connect" << std::endl;
            return;
        }
        struct pollfd clientPollFd;
        std::memset(&clientPollFd, 0, sizeof(clientPollFd));
        clientPollFd.fd = clientSocket;
        clientPollFd.events = POLLIN;
        _DescriptorsPoll.push_back(clientPollFd);

        _clientsBySocket[clientSocket] = Client(clientSocket);

        std::cout << "Accepted Client: " << _numPollDescriptors << std::endl;

        _numPollDescriptors++;
    }
}

// Parse incoming data from a client
// combining the functions _processClientData 
void Server::_processClientData(int fd) //done
{
    Client& client = _clientsBySocket[fd];

    while (true)
    {
        char buffer[MAX_MESSAGE_LENGHT] = {0};
        // read incoming msg
        int received_bytes = recv(fd, buffer, MAX_MESSAGE_LENGHT, 0);

        if (received_bytes == 0)
        {
            std::cout << "Client disconnected" << std::endl;
            _deleteClient(&client);
            return;
        }

        if (received_bytes < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                break;
            }
            else
            {
                std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
                _deleteClient(&client);
                return;
            }
        }

        std::cout << "Received data from client: " << buffer << std::endl;
        client.storeInInputBuffer(buffer);
    }

    while (true)
    {
        // (check if the message is bigger than 512) -> send error
        if (client.exceedsMessageLengthLimit())
        {
            client.queueResponseMessage("417 * :Input line was too long\r\n");
            return;
        }

        // Split the input buffer into lines using '\n' as the delimiter
        std::string clientBuffer = client.obtainInputBuffer();
        size_t pos = clientBuffer.find('\n');
        if (pos == std::string::npos)
        {
            //std::cout << "Incomplete message in buffer: " << buffer << std::endl;
            return;
        }

        std::string message = clientBuffer.substr(0, pos);
        client.truncateInputBuffer(pos + 1); // Remove the processed message from the buffer

        std::cout << "Processing message: " << message << std::endl;

        std::string command;
        std::vector<std::string> params;
        std::istringstream iss(message);
        std::string token;

        // Check for prefix and remove it
        if (message[0] == ':')
        {
            iss >> token; // Read and discard the prefix
        }

        // Extract the command
        iss >> command;

        // Extract the parameters
        while (iss >> token)
        {
            if (token[0] == ':')
            {
                // Extract the trailing part
                std::string trailing;
                std::getline(iss, trailing);
                params.push_back(token.substr(1) + trailing);
                break;
            }
            else
            {
                params.push_back(token);
            }
        }

        // Remove trailing \r from the last parameter
        if (!params.empty() && !params[params.size() - 1].empty() && params[params.size() - 1][params[params.size() - 1].size() - 1] == '\r')
        {
            params[params.size() - 1].resize(params[params.size() - 1].size() - 1);
        }

        std::cout << "Command: " << command << std::endl;
        for (size_t i = 0; i < params.size(); ++i)
        {
            std::cout << "Param " << i << ": " << params[i] << std::endl;
        }

        if (command == std::string("PASS"))
            _handle_pass(&client, params);
        else if (command == std::string("NICK"))
            _handle_nick(&client, params);
        else if (command == std::string("USER"))
            _handle_user(&client, params);
        else if (command == std::string("PING"))
            _handle_ping(&client, params);
        else if (command == std::string("CAP"))
            _handle_cap(&client, params);
        else if (command == std::string("JOIN"))
            _handle_join(&client, params);
        else if (command == std::string("WHO"))
            _handle_who(&client, params);
        else if (command == std::string("PRIVMSG"))
            _handle_privmsg(&client, params);
        else if (command == std::string("QUIT"))
            _handle_quit(&client, params);
        else if (command == std::string("MODE"))
            _handle_mode(&client, params);
        else if (command == std::string("KICK"))
            _handle_kick(&client, params);
        else if (command == std::string("INVITE"))
            _handle_invite(&client, params);
        else if (command == std::string("TOPIC"))
            _handle_topic(&client, params);
        else
        {
            client.queueResponseMessage(std::string("421") + std::string(" * ") + command + std::string(" :Unknown command\r\n"));
        }
    }
}

// updated to avoid memory leaks  
std::vector<std::string> Server::_tokenizeString(const std::string& input, char separator)
{
	std::vector<std::string> result;
	std::string fragment;
	std::istringstream stream(input);
	
	while (std::getline(stream, fragment, separator))
	{
		result.push_back(fragment);
	}

	return result;
}

// updated
void Server::_handleClientDisconnection(Client* user, std::string exitMessage)
{
    std::cout << "Disconnecting client: " << user->_obtainNickname() << std::endl;

    // Send a quit message to all channels the client is in + remove client from all channels it is in
    std::string quitNotification = ":" + user->_obtainNickname() + "!~" + user->_obtainUsername() + " QUIT :" + exitMessage + "\r\n";
    _transmit_to_all_connected_channels(user, quitNotification);

    // Remove client from all joined channels
    std::map<std::string, Channel*> clientChannels = user->get_connected_channels();
    for (std::map<std::string, Channel*>::iterator channelIterator = clientChannels.begin(); channelIterator != clientChannels.end(); ++channelIterator)
    {
        Channel* currentChannel = channelIterator->second;
        currentChannel->removeMember(user);
    }

    // Remove the client from the taken username list
    int clientSocket = user->get_fd();
    if (!user->_obtainNickname().empty())
    {
        std::string clientNickname = _clientsBySocket[clientSocket]._obtainNickname();
        for (std::vector<std::string>::iterator nameIterator = _registeredUsernames.begin(); nameIterator != _registeredUsernames.end(); ++nameIterator)
        {
            if (*nameIterator == clientNickname)
            {
                _registeredUsernames.erase(nameIterator);
                break;
            }
        }
    }

    // Remove the client from the _clientsBySocket map
    _clientsBySocket.erase(clientSocket);

    // Remove the corresponding pollfd from the _DescriptorsPoll vector
    for (std::vector<struct pollfd>::iterator pollIterator = _DescriptorsPoll.begin(); pollIterator != _DescriptorsPoll.end(); ++pollIterator)
    {
        if (pollIterator->fd == clientSocket)
        {
            _DescriptorsPoll.erase(pollIterator);
            break;
        }
    }

    _numPollDescriptors--;
    // Close the client's file descriptor
    close(clientSocket);

    std::cout << "Client disconnected successfully" << std::endl;
}

// check from here
// Disconnect a client
void Server::_disconnectClient(Client* user, std::string exitMessage) {
    _transmit_to_all_connected_channels(user, exitMessage);
    _deleteClient(user->getSocket());
}

// Broadcast a message to all channels a client has joined
// errors fixed
void Server::_transmit_to_all_connected_channels(Client *user, const std::string& msg)
{
	std::map<std::string, Channel*> activeChannels  = user->get_connected_channels();
	for (std::map<std::string, Channel*>::iterator it = activeChannels .begin(); it != activeChannels .end(); ++it)
	{
		Channel* channel = it->second;
		_distribute_msg_to_channel_members(user, channel, msg, false);
	}
}

// Send a message to all members of a channel
// errors fixed
void Server::_distribute_msg_to_channel_members(Client *sender, Channel *channel, const std::string& msg, bool includeSender)
{
	// Get the clients in the channel
	const std::vector<Client*>& members = channel->getMembers();

	// Iterate through the clients and send the message to each of them
	for (std::vector<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		Client* recipient  = *it;
		if (sender != recipient  || includeSender)
			recipient ->queueResponseMessage(msg);
	}
}

// Check if a nickname is valid
// updated
bool Server::_checkNicknameValid(const std::string& nickname)
{
	// Check for lenght 9
	if (nickname.empty() || nickname.length() > 9)
		return false;

	// Check if the first character is a letter or special character
	if (!std::isalpha(nickname[0] && nickname[0] != '_' && nickname[0] != '-' && !isalnum(nickname[0])))
		return false;

	// valid nickname
	return true;
} 

// Check if a username is already in use
// errors fixed
bool Server::_isUsernameTaken(const std::string& username)
{
	for (std::vector<std::string>::const_iterator it = _registeredUsernames.begin(); it != _registeredUsernames.end(); ++it)
	{
		if (*it == username)
		{
			return true;
		}
	}
	return false;
}


// Send a message to all members of a channel
// new version
void Server::_distributeMessageToChannelMembers(const std::string& msg) 
{
	for (std::map<int, Client>::iterator it = _clientsBySocket.begin(); it != _clientsBySocket.end(); ++it)
	{
		Client* sender = &(it->second);
		sender->queueResponseMessage(msg);
	}
}

// send a message to all members of a channel
// new version
void Server::_notifyAllSubscribedChannels(const std::string& message) {
    for (std::map<int, Client>::iterator it = _clientsBySocket.begin(); it != _clientsBySocket.end(); ++it) {
        Client* user = &it->second;
        user->queueResponseMessage(message);
    }
}

//welcome new user
// new version
void	Server::_sendWelcomeMessage(Client* user)
{
	// std::string nickname = user->_obtainNickname();
	// std::string username = user->_obtainUsername();
	// std::string realname = user->_obtainRealname();
	// std::string server_host = _serverName;

	// Send RPL_WELCOME: 001
	std::string rpl_welcome_msg = "001 " + user->_obtainNickname() + " :Welcome to the Internet Relay Network " + user->_obtainNickname() + "!" + user->_obtainUsername() + "@" +  user->_obtainRealname() + "\r\n";
	user->queueResponseMessage(rpl_welcome_msg);

	// Send RPL_YOURHOST: 002
	std::string rpl_yourhost_msg = "002 " + user->_obtainNickname() + " :Your host is " + _serverName + ", running version 1.0\r\n";
	user->queueResponseMessage(rpl_yourhost_msg);

	// Send RPL_CREATED: 003
	std::string rpl_created_msg = "003 " + user->_obtainNickname() + " :This server was created " + _serverCeationTime + "\r\n";
	user->queueResponseMessage(rpl_created_msg);

	// Send RPL_MYINFO: 004
	std::string rpl_myinfo_msg = "004 " + user->_obtainNickname() + " " + _serverName + " 1.0 o o\r\n";
	user->queueResponseMessage(rpl_myinfo_msg);

	user->proceed_registration_status();
}

// Find a client by nickname
Client* Server::_locateClientByNickname(const std::string& nickname) {
    for (std::map<int, Client>::iterator it = _clientsBySocket.begin(); it != _clientsBySocket.end(); ++it) {
        if (it->second._obtainNickname() == nickname) {
            return &it->second;
        }
    }
    return NULL;
}

void Server::eliminateDuplicateEntries(std::vector<std::string>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        for (size_t j = i + 1; j < vec.size();) {
            if (vec[i] == vec[j]) {
                vec.erase(vec.begin() + j);
            } else {
                ++j;
            }
        }
    }
}

// Remove a client from the server
// new version
void Server::_deleteClient(Client* clientFd)
{
	_handleClientDisconnection(clientFd, "Client disconnected unexpectedly");
}
