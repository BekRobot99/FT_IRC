#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sstream>
#include "server.hpp"

bool check_args(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Error: Invalid number of arguments." << std::endl;
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return false;
    }

    // Validate port number
    std::string port_str(argv[1]);
    if (port_str.find_first_not_of("0123456789") != std::string::npos)
    {
        std::cerr << "Error: Port number must contain only digits." << std::endl;
        return false;
    }

    int port = std::atoi(port_str.c_str());
    if (port < 1024 || port > 65535)
    {
        std::cerr << "Error: Port number must be in the range [1024, 65535]." << std::endl;
        return false;
    }

    // Validate password (non-empty)
    std::string password(argv[2]);
    if (password.empty())
    {
        std::cerr << "Error: Password cannot be empty." << std::endl;
        return false;
    }

    return true;
}

// Function to log server startup information
void log_server_startup(int port, const std::string& password) {
    std::time_t now = std::time(0);
    std::tm* local_time = std::localtime(&now);

    std::cout << "=============================================" << std::endl;
    std::cout << "IRC Server Starting..." << std::endl;
    std::cout << "Time: " << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Password: " << password << std::endl;
    std::cout << "=============================================" << std::endl;
}

int main(int argc, char **argv)
{
    // Check for correct number of arguments
    if (!check_args(argc, argv))
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return EXIT_FAILURE;
    }

    // Parse command line arguments
    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    log_server_startup(port, password);

    try {
    // Create and run the server
    Server server(port, password, "MyIRCServer"); // Add a default server name
    server.startRun();
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
}

    return EXIT_SUCCESS;
}
