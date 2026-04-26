// server.cpp - Multi-threaded TCP server with graceful shutdown

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <atomic>   // for std::atomic<bool>
#include <csignal>  // for signal(), SIGINT, SIGTERM

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include "src/kvstore.h"
#include "src/persistence.h"

// -------------------------------------------------------
// GLOBAL FLAG: is the server shutting down?
// -------------------------------------------------------
// std::atomic<bool> is thread-safe — multiple threads can
// read it at the same time without a mutex.
// When this flips to false, threads stop accepting/reading.

std::atomic<bool> running(true);

// -------------------------------------------------------
// GLOBAL: server socket fd
// -------------------------------------------------------
// We need this in the signal handler so we can close it,
// which causes accept() to unblock and the loop to exit.

int server_fd = -1;

// -------------------------------------------------------
// SIGNAL HANDLER: called when Ctrl+C or kill is received
// -------------------------------------------------------
// SIGINT  = Ctrl+C
// SIGTERM = kill command (what systemd/docker sends on shutdown)
//
// We just flip the flag and close the server socket.
// The main loop will notice and exit cleanly.

void handle_signal(int signal) {
    std::cout << "\nShutdown signal received (" << signal << "). Shutting down...\n";
    running = false;
    if (server_fd != -1) {
        close(server_fd); // this unblocks accept() in the main loop
    }
}

// -------------------------------------------------------
// HELPER: Split "SET name sreekar" into ["SET","name","sreekar"]
// -------------------------------------------------------
std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

// -------------------------------------------------------
// HELPER: Parse command and call the right KVStore method
// -------------------------------------------------------
std::string handle_command(const std::string& raw, KVStore& store) {
    auto parts = split(raw);

    if (parts.empty()) return "ERROR: empty command\n";

    const std::string& cmd = parts[0];

    if (cmd == "SET") {
        if (parts.size() < 3) return "ERROR: SET requires key and value\n";
        store.put(parts[1], parts[2]);
        return "OK\n";

    } else if (cmd == "GET") {
        if (parts.size() < 2) return "ERROR: GET requires key\n";
        auto result = store.get(parts[1]);
        if (result.has_value()) return result.value() + "\n";
        else                    return "NULL\n";

    } else if (cmd == "DEL") {
        if (parts.size() < 2) return "ERROR: DEL requires key\n";
        store.remove(parts[1]);
        return "OK\n";

    } else {
        return "ERROR: unknown command '" + cmd + "'\n";
    }
}

// -------------------------------------------------------
// HELPER: Handle one client — runs in its own thread
// -------------------------------------------------------
void handle_client(int client_fd, KVStore& store) {
    char buffer[1024];

    while (running) {  // ← check the flag on every iteration
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read <= 0) {
            std::cout << "Client disconnected.\n";
            break;
        }

        std::cout << "Received: " << buffer;
        std::string response = handle_command(std::string(buffer), store);
        write(client_fd, response.c_str(), response.size());
    }

    close(client_fd);
}

int main() {

    // -------------------------------------------------------
    // Register signal handlers BEFORE doing anything else
    // -------------------------------------------------------
    // Now Ctrl+C and kill both call handle_signal()
    // instead of instantly killing the process

    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    // Create shared KVStore and Persistence
    KVStore store(100);
    Persistence persistence("fastkvs_data.txt");

    // Load previously saved data on startup if file exists
    if (persistence.exists()) {
        persistence.load(store);
        std::cout << "Data loaded from disk.\n";
    }

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }
    std::cout << "Socket created\n";

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(7379);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Bind failed\n";
        return 1;
    }
    std::cout << "Bound to port 7379\n";

    if (listen(server_fd, 5) == -1) {
        std::cerr << "Listen failed\n";
        return 1;
    }
    std::cout << "Listening on port 7379...\n";
    std::cout << "Server ready. Waiting for clients...\n";

    // Accept loop
    while (running) {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        int client_fd = accept(server_fd, (sockaddr*)&client_address, &client_len);

        if (client_fd == -1) {
            // If we're shutting down, accept() fails — that's expected
            if (!running) break;
            std::cerr << "Accept failed, continuing...\n";
            continue;
        }

        std::cout << "Client connected!\n";
        std::thread(handle_client, client_fd, std::ref(store)).detach();
    }

    // -------------------------------------------------------
    // GRACEFUL SHUTDOWN — we only reach here after Ctrl+C
    // -------------------------------------------------------
    std::cout << "Saving data to disk...\n";
    persistence.save(store);
    std::cout << "Data saved. Goodbye.\n";

    return 0;
}
