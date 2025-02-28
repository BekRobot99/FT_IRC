
    #ifndef CLIENT_HPP
    #define CLIENT_HPP

    #include <string>
    #include <map>
    #include <vector>

    #define MAX_MSG_SIZE  512

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
        // Member variables declared in the correct order
        int                             _fd;                                // File descriptor for the client's socket
        std::string                     _clientUsername;                    // Client's username
        std::string                     _clientNickname;                    // Client's nickname
        std::map<std::string, Channel*> _joinedChannels;                    // Channels the client has joined
        Channel*                        _activeChannel;                     // The client's currently active channel
        std::string                     _clientRealname;                    // Client's real name
        std::string                     _inputBuffer;                       // Buffer for incoming messages
        std::string                     _outputBuffer;                      // Buffer for outgoing messages
        std::string                     _responseBuffer;                    // Buffer for responses to the client
        ClientStatus                    _registrationStatus;                // Current registration status
        std::string                     _hostname;                          // Client's hostname
        std::string                     _servername;                        // Server name the client is connected
        std::string                     _buffer;                            // Buffer for stopped clients
        bool                            _is_stopped;                        // Flag to indicate if the client is stopped
        bool                            _is_invisible;                      // Flag to indicate if the client is invisible
        bool                            _is_operator;                       // Flag to indicate if the client is an operator

    public:
        // Constructors and Destructor
        Client();                                                           // Default constructor
        Client(int fd);                                                     // Parameterized constructor
        ~Client();                                                          // Destructor
        Client(const Client& other);                                        // Copy constructor
        Client& operator=(const Client& other);                             // Assignment operator

        // Getters
        std::string                     _obtainNickname() const;            // Get the client's nickname
        std::string                     _obtainUsername() const;            // Get the client's username
        std::string                     _obtainHostname() const;            // Get the client's hostname
        std::string                     _obtainRealname() const;            // Get the client's real name
       std::map<std::string, Channel*>  get_connected_channels();     // Get the channels the client has joined
        int                             getSocket() const;                  // Get the client's file descriptor
        bool                            is_registered() const;              // Check if the client is fully registered
        ClientStatus                    getRegistrationStatus() const;      // Get the client's registration status

        // Setters
        void                            updateRegistrationStatus(); // Update the client's registration status
        void                            updateUsername(std::string& nickname); // Set the client's nickname
        void                            storeUsername(std::string& username); // Set the client's username
        void                            storeRealname(std::string& realname); // Set the client's real name

        // Channel Management
        std::map<std::string, Channel*> getSubscribedChannels() const;     // Get the channels the client has joined
        void                            enterChannel(std::string channelName, Channel *targetChannel); // Enter a channel
        void                            exitChannel(const std::string& channelName); // Exit a channel

        // Buffer Management
        void                            queueResponseMessage(std::string message); // Queue a response message
        void                            clear_buffer();                     // Clear the buffer

        // Status Management
        void                            set_stopped(bool value);            // Set the stopped flag
        bool                           is_stopped() const { return _is_stopped; }
        void                            set_invisible(bool value) { _is_invisible = value; }
        void                            set_operator(bool value) { _is_operator = value; }
        void                            append_to_buffer(const std::string& data) { _buffer += data;}
        std::string                     get_buffer() const { return _buffer;}
        std::string                     obtainInputBuffer();                // Get the input buffer
        void                             storeInInputBuffer(char* buffer); // Store data in the input buffer
        void                            truncateInputBuffer(size_t pos);   // Truncate the input buffer
        bool                          is_incoming_msg_complete() const;  // Check if an incoming message is complete
        bool                          is_incoming_msg_too_long() const;  // Check if an incoming message is too long
        bool                          is_response_complete() const;      // Check if a response is complete
        short                         send_out_buffer();                 // Send the output buffer


    };

    #endif
