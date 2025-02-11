#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <map>
#include <vector>

class Channel;
// Enum to represent the client's registration status
enum ClientStatus {
    STATUS_NICK,       // Client has sent the NICK command
    STATUS_USER,       // Client has sent the USER command
    STATUS_PASS,       // Client has sent the PASS command
    STATUS_REGISTERED  // Client has completed registration
};
class Client {
private:
    int                             _fd;                                // File descriptor for the client's socket
    std::string                     _clientUsername;                          // Client's username
    std::string                     _clientNickname;                          // Client's nickname
    std::map<std::string, Channel*> _joinedChannels;                   // Channels the client has joined
    Channel*                        _activeChannel;                    // The client's currently active channel
    std::string                     _clientRealname;                          // Client's real name
    std::string                     _inputBuffer;                         // Buffer for incoming messages
    std::string                     _outputBuffer;                        // Buffer for outgoing messages
    std::string                     _responseBuffer;                   // Buffer for responses to the client
    ClientStatus                    _registrationStatus;               // Current registration status
    std::string                     _hostname;                          // Client's hostname
    std::string                     _servername;                        // Server name the client is connected 
public:
    Client();                                                           // Default constructor
    Client(int fd);                                                     // Parameterized constructor
    ~Client();                                                          // Destructor
    Client(const Client& other);                                        // Copy constructor
    Client& operator=(const Client& other);                             // Assignment operator
    bool                           is_registered() const;               // Check if the client is fully registered
    
    std::string                     _obtainNickname() const;               // Get the client's nickname
    std::map<std::string, Channel*> get_connected_channels() const;        // Get the channels the client has joined
    int                             getSocket() const;                     // Get the client's file descriptor
    void                            updateUsername(const std::string& newUsername); // Set the client's nickname
private:
    std::string _buffer; // Buffer for stopped clients
    bool _is_stopped;    // Flag to indicate if the client is stopped

    public:
        Client(int fd) : _fd(fd), _is_stopped(false) {}

        void set_stopped(bool value) { _is_stopped = value; }
        bool is_stopped() const { return _is_stopped; }

        void append_to_buffer(const std::string& data) {
            _buffer += data;
        }

        std::string get_buffer() const {
            return _buffer;
        }

        void clear_buffer() {
            _buffer.clear();
        }
        
};
#endif