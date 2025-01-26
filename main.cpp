#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sstream>

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

int main(int argc, char **argv)
{
    // Check for correct number of arguments
    if (!check_args(argc, argv))
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
