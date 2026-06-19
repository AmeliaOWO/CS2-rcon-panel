#pragma once
#include "rcon_client.h"
#include "settings.h"
#include <string>
#include <vector>
#include <deque>
#include <map>

// ── App state shared between UI and main loop ────────────────────────────────
struct AppState {
    // Connection
    std::string conn_addr   = "127.0.0.1:27015";  // full address:port typed by user
    std::string conn_ip     = "127.0.0.1";          // parsed host
    int         conn_port   = 27015;                // parsed port
    std::string conn_pass;
    bool        connecting   = false;
    std::string conn_error;             // last connection error message

    // Active tab
    int         active_tab   = 0;

    // Font indices
    int         font_large   = 0;       // index of the 22px title font

    // Console
    std::deque<std::string> console_lines;
    std::string             console_input;
    bool                    scroll_console = true;
    int                     max_console_lines = 500;
    std::string             console_filter;       // keyword filter for console

    // Command history (up/down arrows)
    std::vector<std::string> cmd_history;
    int                      cmd_history_pos = -1;

    // Player / server info
    struct PlayerInfo {
        std::string name;
        int         score  = 0;
        int         kills  = 0;
        int         deaths = 0;
        int         ping   = 0;
        std::string steamid;
    };
    std::vector<PlayerInfo> players;
    std::string             map_name;
    int                     player_count  = 0;
    int                     max_players   = 0;
    int                     ct_score      = 0;
    int                     t_score       = 0;

    // Last auto-refresh timestamp
    double last_refresh = 0.0;

    // RCON client
    RCONClient* rcon = nullptr;

    // User input buffers
    std::string say_buffer;
    std::string kick_buffer;

    // Per-cvar string input buffers (key = cvar name)
    std::map<std::string, std::string> str_buffers;

    // Server address history (saved to disk)
    std::vector<std::string> server_history;
    int                      max_history = 20;
    std::map<std::string, std::string> server_passwords;  // addr → password

    // Server hostname (from "status" output)
    std::string server_hostname;

    // Saved custom presets list
    std::vector<std::string> saved_presets;
    std::string              new_preset_name;    // buffer for naming a new preset
};

// ── Main UI entry point ─────────────────────────────────────────────────────
void render_ui(AppState& state);
void add_console_line(AppState& state, const std::string& line);
void parse_server_status(AppState& state, const std::string& status);

// ── Server history persistence ───────────────────────────────────────────────
void load_server_history(AppState& state);
void save_server_history(const AppState& state);
