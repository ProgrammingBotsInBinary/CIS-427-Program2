// Basic C++ libraries
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

// Socket programming libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Server port
const int server_port = 3976;

// Maximum line size
const int max_output = 256;

// Main function
int main(int argc, char* argv[]) {
    
    // Host information gathered by user
    struct hostent* host_info;
    
    // Server address
    struct sockaddr_in server_address;
    
    // Set the buffer
    char buffer[max_output];
    
    // Initialize the socket and connection status
    int client_socket;
    int connection_result;

    // Check if host is provided in the main argument
    if (argc != 2) {
        std::cerr << "Error: No host address provided!\nPlease use: ./client <host>\n";
        return EXIT_FAILURE;
    }

    // Translate host name into IP address
    host_info = gethostbyname(argv[1]);
    
    // Check if the host connection is successful
    if (!host_info) {
        std::cerr << "Error: Unknown host!\n";
        return EXIT_FAILURE;
    }

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // Check if the socket failed to open
    if (client_socket < 0) {
        std::cerr << "Error: Failed to open socket!\n";
        return EXIT_FAILURE;
    }

    // Initialize server address data structure
    bzero((char*)&server_address, sizeof(server_address));
    
    // Server address is an internet address
    server_address.sin_family = AF_INET;
    
    // Copy the host address to the server address
    bcopy(host_info->h_addr, (char*)&server_address.sin_addr, host_info->h_length);
    
    // Set the port to the port (defined)
    server_address.sin_port = htons(server_port);
    
    // Set the address to the correct size
    memset(&server_address.sin_zero, 0, 8);

    // Connect to the server
    connection_result = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    
    //If the server connection has failed
    if (connection_result < 0) {
        std::cerr << "Error: Could not connect to the server!\n";
        close(client_socket);
        return EXIT_FAILURE;
    }

    // If is hasn't failed, the client will continue
    std::cout << "Successfully connected to the server!\n";

    // Receive the message from the server
    recv(client_socket, buffer, max_output, 0);
    
    // Output the server message
    std::cout << buffer << std::endl;

    // While the program is still running
    while (std::cout << "CLIENT> ") {
        // Read input from user
        fgets(buffer, max_output, stdin);

        // Save a copy of the last command for later comparison
        char last_command[max_output];
        strncpy(last_command, buffer, sizeof(buffer));

        // Send the message to the server
        send(client_socket, buffer, max_output, 0);

        // Receive the response from the server
        recv(client_socket, buffer, max_output, 0);
        
        // Output the buffer
        std::cout << "CLIENT> " << buffer << std::endl << std::endl;
      
        // If quit is commanded
        if (strcmp(last_command, "QUIT\n") == 0) {
            close(client_socket); // Close the socket
            std::cout << "Closed socket: " << client_socket << std::endl;
            
            exit(EXIT_SUCCESS); // Exit program
        }
    }
}
