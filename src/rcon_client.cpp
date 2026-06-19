#include "rcon_client.h"
#include <cstring>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#define CLOSE_SOCK(fd)  closesocket(fd)
#else
#define CLOSE_SOCK(fd)  ::close(fd)
#endif

// ── Helpers ──────────────────────────────────────────────────────────────────
static inline void pack32(char* buf, int32_t v) noexcept {
    buf[0] =  v        & 0xFF;
    buf[1] = (v >> 8)  & 0xFF;
    buf[2] = (v >> 16) & 0xFF;
    buf[3] = (v >> 24) & 0xFF;
}

static inline int32_t unpack32(const char* buf) noexcept {
    return  (int32_t)(buf[0] & 0xFF)
          | (int32_t)(buf[1] & 0xFF) << 8
          | (int32_t)(buf[2] & 0xFF) << 16
          | (int32_t)(buf[3] & 0xFF) << 24;
}

// ── Constructor / Destructor ─────────────────────────────────────────────────
RCONClient::RCONClient() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

RCONClient::~RCONClient() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

// ── Connect ──────────────────────────────────────────────────────────────────
bool RCONClient::connect(const std::string& host, uint16_t port,
                         const std::string& password) {
    if (connected_) disconnect();

    host_     = host;
    port_     = port;
    password_ = password;
    server_info_ = host_ + ":" + std::to_string(port_);
    packet_id_ = 0;

    // Create socket
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET) {
        return false;
    }

    // Set receive timeout (so recv_packet doesn't block forever)
#ifdef _WIN32
    int rcvtimeo = 3000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&rcvtimeo, sizeof(rcvtimeo));
    int sndtimeo = 3000;
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sndtimeo, sizeof(sndtimeo));
#else
    struct timeval tv{3, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif

    // Resolve address
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = INADDR_NONE;

#ifdef _WIN32
    struct addrinfo hints{}, *result = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &result) == 0 && result) {
        addr = *(sockaddr_in*)result->ai_addr;
        addr.sin_port = htons(port);
        freeaddrinfo(result);
    } else
#endif
    {
        addr.sin_addr.s_addr = inet_addr(host.c_str());
    }

    // Connect
    if (::connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        CLOSE_SOCK(fd);
        return false;
    }

    // Success — assign socket and start worker
    sock_ = fd;
    connected_ = true;
    authenticated_ = false;
    running_ = true;
    worker_ = std::thread(&RCONClient::worker_loop, this);

    // Send auth packet immediately
    send_packet(0, RCON_AUTH, password_);

    return true;
}

// ── Disconnect ───────────────────────────────────────────────────────────────
void RCONClient::disconnect() {
    running_ = false;

    // Close socket to wake up worker
    if (sock_ != INVALID_SOCKET) {
        CLOSE_SOCK(sock_);
        sock_ = INVALID_SOCKET;
    }

    if (worker_.joinable()) {
        worker_.join();
    }

    connected_ = false;
    authenticated_ = false;

    {
        std::lock_guard<std::mutex> lk(cmd_mutex_);
        std::queue<std::string>().swap(cmd_queue_);
    }
    {
        std::lock_guard<std::mutex> lk(resp_mutex_);
        responses_.clear();
    }
}

// ── Send Command ─────────────────────────────────────────────────────────────
void RCONClient::send_command(const std::string& cmd) {
    if (!connected_ || !authenticated_) return;
    std::lock_guard<std::mutex> lk(cmd_mutex_);
    cmd_queue_.push(cmd);
}

// ── Consume Responses ────────────────────────────────────────────────────────
std::vector<std::string> RCONClient::consume_responses() {
    std::lock_guard<std::mutex> lk(resp_mutex_);
    auto result = std::move(responses_);
    responses_.clear();
    return result;
}

// ── Send packet ──────────────────────────────────────────────────────────────
bool RCONClient::send_packet(int32_t id, int32_t type, const std::string& body) {
    if (sock_ == INVALID_SOCKET) return false;

    // Size = id(4) + type(4) + body + null(2)
    int32_t pkt_size = static_cast<int32_t>(body.size()) + 10;
    char header[12];
    pack32(header,      pkt_size);
    pack32(header + 4,  id);
    pack32(header + 8,  type);

    // Send header + body + nulls in one go if possible
    std::string wire(header, 12);
    wire += body;
    wire += '\0';
    wire += '\0';

    int sent = ::send(sock_, wire.data(), (int)wire.size(), 0);
    return sent == (int)wire.size();
}

// ── Receive packet ───────────────────────────────────────────────────────────
bool RCONClient::recv_packet(RCONPacket& pkt, int /*timeout_ms*/) {
    if (sock_ == INVALID_SOCKET) return false;

    // Read 4-byte size
    char size_buf[4];
    int n = ::recv(sock_, size_buf, 4, 0);
    if (n <= 0) return false;

    int32_t pkt_size = unpack32(size_buf);
    if (pkt_size < 4 || pkt_size > 65536) return false;

    // Read the rest: id(4) + type(4) + body + 2 nulls.
    // The Valve spec says pkt_size INCLUDES the two trailing null bytes.
    // But some servers don't count them, so we read pkt_size + 2
    // (the extra 2 are nulls which we strip anyway).
    size_t want = (size_t)pkt_size + 2;
    std::vector<char> buf(want);
    int total_read = 0;
    while (total_read < (int)want) {
        n = ::recv(sock_, buf.data() + total_read,
                   (int)(want - total_read), 0);
        if (n <= 0) {
            // If we got most of the data but timed out on the last 2 null bytes,
            // that's OK — the server may not have sent them
            if (total_read >= 8) break;
            return false;
        }
        total_read += n;
    }

    pkt.size = pkt_size;
    pkt.id   = unpack32(buf.data());
    pkt.type = unpack32(buf.data() + 4);
    pkt.body = std::string(buf.data() + 8, total_read - 8);

    // Strip trailing nulls
    while (!pkt.body.empty() && pkt.body.back() == '\0')
        pkt.body.pop_back();

    return true;
}

// ── Worker Loop ──────────────────────────────────────────────────────────────
void RCONClient::worker_loop() {
    auto auth_start = std::chrono::steady_clock::now();
    bool auth_handled = false;

    while (running_ && connected_) {
        // ── Send queued commands ──
        if (authenticated_) {
            std::queue<std::string> local_queue;
            {
                std::lock_guard<std::mutex> lk(cmd_mutex_);
                local_queue.swap(cmd_queue_);
            }
            while (!local_queue.empty()) {
                const auto& cmd = local_queue.front();
                int32_t req_id = next_id();
                send_packet(req_id, RCON_EXECCOMMAND, cmd);
                local_queue.pop();
            }
        }

        // ── Read responses ──
        RCONPacket pkt;
        while (recv_packet(pkt, 50)) {
            if (pkt.type == RCON_AUTH_RESPONSE) {
                authenticated_ = (pkt.id != -1);
                if (on_auth_) on_auth_(authenticated_);
                auth_handled = true;
                {
                    std::lock_guard<std::mutex> lk(resp_mutex_);
                    if (authenticated_)
                        responses_.push_back("[RCON] 认证成功");
                    else
                        responses_.push_back("[RCON] 认证失败（密码错误？）");
                }
            }
            else if (pkt.type == RCON_RESPONSE_VALUE) {
                if (!pkt.body.empty()) {
                    std::lock_guard<std::mutex> lk(resp_mutex_);
                    responses_.push_back(pkt.body);
                }
            }
        }

        // ── Auth timeout (8 seconds) ──
        if (!auth_handled) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - auth_start).count();
            if (elapsed > 8) {
                // Auth timed out — report error, the callback will be
                // triggered after the loop
                break;
            }
        }

        // ── Periodic keep-alive ping (every 30s) ──
        if (authenticated_) {
            static auto last_ping = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(
                    now - last_ping).count() >= 30) {
                send_packet(next_id(), RCON_EXECCOMMAND, "ping");
                last_ping = now;
            }
        }

        // ── Disconnect detection ──
        if (sock_ == INVALID_SOCKET) {
            if (!auth_handled && on_auth_) on_auth_(false);
            if (on_disconnect_) on_disconnect_();
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    connected_ = false;
    authenticated_ = false;

    // Notify UI about auth timeout
    if (!auth_handled) {
        {
            std::lock_guard<std::mutex> lk(resp_mutex_);
            responses_.push_back(
                "[RCON] 连接超时：服务器无响应，请检查地址和端口");
        }
    }
}
