// server.cpp - Multi-threaded TCP server with metrics

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <atomic>
#include <csignal>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include "src/kvstore.h"
#include "src/persistence.h"

// -------------------------------------------------------
// METRICS — atomic counters, one per stat
// -------------------------------------------------------
// std::atomic means multiple threads can increment these
// simultaneously without a mutex — the CPU guarantees
// the increment is one indivisible operation.
//
// If we used a regular int, two threads incrementing at
// the same time could corrupt the value (race condition).

std::atomic<uint64_t> total_commands(0);  // every SET/GET/DEL
std::atomic<uint64_t> total_hits(0);      // GET found the key
std::atomic<uint64_t> total_misses(0);    // GET did not find the key
std::atomic<uint64_t> total_sets(0);      // SET commands
std::atomic<uint64_t> total_deletes(0);   // DEL commands

// -------------------------------------------------------
// GLOBAL FLAG + SERVER FD (for graceful shutdown)
// -------------------------------------------------------
std::atomic<bool> running(true);
int server_fd = -1;

void handle_signal(int signal) {
    std::cout << "\nShutdown signal received (" << signal << "). Shutting down...\n";
    running = false;
}

// -------------------------------------------------------
// HELPER: Split "SET name sreekar" → ["SET","name","sreekar"]
// -------------------------------------------------------
std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

// -------------------------------------------------------
// HELPER: Build STATS response string
// -------------------------------------------------------
// Called when client sends "STATS"
// Reads all atomic counters and formats them into a string

std::string get_stats() {
    uint64_t hits   = total_hits.load();
    uint64_t misses = total_misses.load();
    uint64_t total  = hits + misses;

    // Calculate hit rate — avoid division by zero
    double hit_rate = (total > 0) ? (100.0 * hits / total) : 0.0;

    std::ostringstream oss;
    oss << "total_commands : " << total_commands.load() << "\n"
        << "total_sets     : " << total_sets.load()     << "\n"
        << "total_deletes  : " << total_deletes.load()  << "\n"
        << "get_hits       : " << hits                  << "\n"
        << "get_misses     : " << misses                << "\n"
        << "hit_rate       : " << hit_rate              << "%\n";
    return oss.str();
}

// -------------------------------------------------------
// HELPER: Parse command, update metrics, call KVStore
// -------------------------------------------------------
std::string handle_command(const std::string& raw, KVStore& store) {
    auto parts = split(raw);
    if (parts.empty()) return "ERROR: empty command\n";

    const std::string& cmd = parts[0];

    // Every valid command increments total_commands
    // ++ on an atomic is thread-safe — no mutex needed

    if (cmd == "SET") {
        if (parts.size() < 3) return "ERROR: SET requires key and value\n";
        store.put(parts[1], parts[2]);
        total_commands++;
        total_sets++;
        return "OK\n";

    } else if (cmd == "GET") {
        if (parts.size() < 2) return "ERROR: GET requires key\n";
        auto result = store.get(parts[1]);
        total_commands++;
        if (result.has_value()) {
            total_hits++;           // key was found — hit
            return result.value() + "\n";
        } else {
            total_misses++;         // key not found — miss
            return "NULL\n";
        }

    } else if (cmd == "DEL") {
        if (parts.size() < 2) return "ERROR: DEL requires key\n";
        store.remove(parts[1]);
        total_commands++;
        total_deletes++;
        return "OK\n";

    } else if (cmd == "STATS") {
        // STATS doesn't count as a command in the metrics
        // — it's a read of the metrics themselves
        return get_stats();

    } else {
        return "ERROR: unknown command '" + cmd + "'\n";
    }
}

// -------------------------------------------------------
// HELPER: Read one complete line from a TCP socket
// -------------------------------------------------------
// TCP is a streaming protocol — one send() on the client
// side does not guarantee one recv() on the server side.
// Data can arrive in chunks. We loop reading one byte at
// a time until we see '\n' which marks end of command.
std::string read_line(int fd) {
    std::string line;
    char c;
    while (true) {
        int n = read(fd, &c, 1);
        if (n <= 0) return ""; // client disconnected or error
        if (c == '\n') break;  // full command received
        if (c != '\r') line += c; // skip \r for Windows clients
    }
    return line;
}

// -------------------------------------------------------
// HELPER: Handle one client — runs in its own thread
// -------------------------------------------------------
void handle_client(int client_fd, KVStore& store) {

    while (running) {
        std::string line = read_line(client_fd);

        if (line.empty()) {
            std::cout << "Client disconnected.\n";
            break;
        }

        std::cout << "Received: " << line << "\n";
        std::string response = handle_command(line, store);
        ssize_t written = write(client_fd, response.c_str(), response.size());
        if (written < 0) {
            std::cerr << "Write failed: " << strerror(errno) << "\n";
            break; // exit the read loop, close connection
        }
    }

    close(client_fd);
}

int main() {
    std::cout.setf(std::ios::unitbuf);
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    KVStore store(100);
    Persistence persistence("fastkvs_data.txt");

    if (persistence.exists()) {
        persistence.load(store);
        std::cout << "Data loaded from disk.\n";
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { std::cerr << "Failed to create socket\n"; return 1; }
    std::cout << "Socket created\n";

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(7379);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Bind failed\n"; return 1;
    }
    std::cout << "Bound to port 7379\n";

    if (listen(server_fd, 5) == -1) {
        std::cerr << "Listen failed\n"; return 1;
    }
    std::cout << "Listening on port 7379...\n";
    std::cout << "Server ready. Waiting for clients...\n";

    while (running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        int ready = select(server_fd + 1, &read_fds, nullptr, nullptr, &tv);
        if (ready <= 0) continue; // timeout or error, check running flag
                                  //
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        int client_fd = accept(server_fd, (sockaddr*)&client_address, &client_len);
        if (client_fd == -1) {
            if (!running) break;
            std::cerr << "Accept failed, continuing...\n";
            continue;
        }

        std::cout << "Client connected!\n";
        std::thread(handle_client, client_fd, std::ref(store)).detach();
    }

    // Graceful shutdown
    std::cout << "Saving data to disk...\n";
    persistence.save(store);
    std::cout << "Data saved. Goodbye.\n";

    return 0;
}
