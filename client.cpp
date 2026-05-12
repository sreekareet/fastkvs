// client.cpp - A bare minimum TCP client
// Goal: connect to our server, send a message, print the response

#include <iostream>
#include <string>

#include <sys/socket.h>  // socket(), connect()
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // inet_pton() — converts "127.0.0.1" string to binary
#include <unistd.h>      // close(), read(), write()
#include <cstring>       // memset()

int main() {

    // -------------------------------------------------------
    // STEP 1: Create a socket (same as server side)
    // -------------------------------------------------------
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }


    // -------------------------------------------------------
    // STEP 2: Specify WHO to connect to
    // -------------------------------------------------------
    // We want to connect to 127.0.0.1 (localhost) on port 7379
    // 127.0.0.1 just means "this same machine"

    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port   = htons(7379);

    // inet_pton converts the string "127.0.0.1" into the binary
    // format that the socket API expects
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);


    // -------------------------------------------------------
    // STEP 3: Connect to the server
    // -------------------------------------------------------
    // This is the "dial the phone number" step.
    // If the server isn't running, this will fail.

    if (connect(sock_fd, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Connection failed — is the server running?\n";
        return 1;
    }
    std::cout << "Connected to server!\n";


    // -------------------------------------------------------
    // STEP 4: Send multiple messages
    // -------------------------------------------------------
    // We send 3 commands one by one, reading the response
    // after each one — like a real back-and-forth conversation

    std::string commands[] = {
        "SET name sreekar\n",
        "SET city toronto\n",
        "GET name\n",
        "GET unknown\n",   // this will be a miss
        "DEL city\n",
        "STATS\n"          // fetch metrics at the end
    };

    char buffer[1024];

    for (const auto& cmd : commands) {
        // Send the command
        write(sock_fd, cmd.c_str(), cmd.size());
        std::cout << "Sent: " << cmd;

        // Read the response
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(sock_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            std::cout << "Server said: " << buffer;
        }
    }


    // -------------------------------------------------------
    // STEP 6: Clean up
    // -------------------------------------------------------
    close(sock_fd);
    std::cout << "Done.\n";
    return 0;
}
