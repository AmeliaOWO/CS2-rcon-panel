#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <queue>
#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
using SOCKET = int;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#endif

// ── RCON Packet Types ────────────────────────────────────────────────────────
enum RCONType : int32_t {
    RCON_RESPONSE_VALUE  = 0,
    RCON_EXECCOMMAND     = 2,
    RCON_AUTH_RESPONSE   = 2,
    RCON_AUTH            = 3
};

struct RCONPacket {
    int32_t size = 0;
    int32_t id   = 0;
    int32_t type = 0;
    std::string body;
};

// ── RCON Client ──────────────────────────────────────────────────────────────
class RCONClient {
public:
    RCONClient();
    ~RCONClient();

    bool connect(const std::string& host, uint16_t port, const std::string& password);
    void disconnect();
    bool is_connected() const noexcept { return connected_; }
    bool is_authenticated() const noexcept { return authenticated_; }
    std::string server_info() const noexcept { return server_info_; }

    // Push a command for the worker to send
    void send_command(const std::string& cmd);

    // Retrieve & clear accumulated responses
    std::vector<std::string> consume_responses();

    // Callbacks
    void set_on_auth(std::function<void(bool)> cb) { on_auth_ = std::move(cb); }
    void set_on_disconnect(std::function<void()> cb) { on_disconnect_ = std::move(cb); }

private:
    void worker_loop();
    bool send_packet(int32_t id, int32_t type, const std::string& body);
    bool recv_packet(RCONPacket& pkt, int timeout_ms = 2000);
    void flush_responses();
    int32_t next_id() noexcept { return ++packet_id_; }

    SOCKET sock_ = INVALID_SOCKET;
    std::thread worker_;
    std::mutex cmd_mutex_;
    std::mutex resp_mutex_;

    std::atomic<bool> connected_{false};
    std::atomic<bool> authenticated_{false};
    std::atomic<bool> running_{false};

    std::queue<std::string> cmd_queue_;
    std::vector<std::string> responses_;

    std::string host_;
    uint16_t port_ = 27015;
    std::string password_;
    std::string server_info_;
    std::atomic<int32_t> packet_id_{0};

    std::function<void(bool)> on_auth_;
    std::function<void()> on_disconnect_;

    // Buffer for assembling partial packets
    std::vector<char> recv_buf_;
};
