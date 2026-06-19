#include "ui.h"
#include "theme.h"
#include "imgui.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <fstream>

// ── Helpers ──────────────────────────────────────────────────────────────────

static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void SendCmd(AppState& state, const std::string& cmd) {
    if (state.rcon && state.rcon->is_authenticated()) {
        state.rcon->send_command(cmd);
        add_console_line(state, "> " + cmd);
    }
}

// ── Server address history ───────────────────────────────────────────────────

static std::string history_file() {
    // Save alongside the executable
    char buf[1024] = {};
#ifdef _WIN32
    GetModuleFileNameA(nullptr, buf, sizeof(buf) - 1);
    char* last = strrchr(buf, '\\');
    if (last) *last = '\0';
    return std::string(buf) + "\\server_history.txt";
#else
    return "server_history.txt";
#endif
}

void load_server_history(AppState& state) {
    state.server_history.clear();
    state.server_passwords.clear();
    std::ifstream f(history_file());
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        // Format: "addr:port" (old) or "addr:port|password" (new)
        auto pipe = line.find('|');
        std::string addr = (pipe != std::string::npos) ? line.substr(0, pipe) : line;
        std::string pass = (pipe != std::string::npos) ? line.substr(pipe + 1) : "";
        state.server_history.push_back(addr);
        if (!pass.empty()) state.server_passwords[addr] = pass;
    }
    // Keep newest first
    std::reverse(state.server_history.begin(), state.server_history.end());
}

void save_server_history(const AppState& state) {
    std::ofstream f(history_file());
    if (!f.is_open()) return;
    f << "# CS2 RCON Server History (addr:port|password)\n";
    // Save newest first (reverse order)
    for (int i = (int)state.server_history.size() - 1; i >= 0; --i) {
        const auto& addr = state.server_history[i];
        f << addr;
        auto it = state.server_passwords.find(addr);
        if (it != state.server_passwords.end() && !it->second.empty())
            f << "|" << it->second;
        f << "\n";
    }
    f << "# EOF\n";
}

static void add_history(AppState& state, const std::string& addr) {
    if (addr.empty()) return;
    // Remove if already exists
    auto it = std::find(state.server_history.begin(),
                        state.server_history.end(), addr);
    if (it != state.server_history.end())
        state.server_history.erase(it);
    // Add to front
    state.server_history.push_back(addr);
    // Save current password
    if (!state.conn_pass.empty())
        state.server_passwords[addr] = state.conn_pass;
    // Trim
    while ((int)state.server_history.size() > state.max_history)
        state.server_history.erase(state.server_history.begin());
    // Save
    save_server_history(state);
}

// ── Console ──────────────────────────────────────────────────────────────────

void add_console_line(AppState& state, const std::string& line) {
    state.console_lines.push_back(line);
    while ((int)state.console_lines.size() > state.max_console_lines)
        state.console_lines.pop_front();
    state.scroll_console = true;
}

// ── Simple "status" / "stats" parser ────────────────────────────────────────
void parse_server_status(AppState& state, const std::string& status) {
    std::istringstream ss(status);
    std::string line;
    while (std::getline(ss, line)) {
        auto cpos = line.find(':');
        if (cpos == std::string::npos) continue;

        std::string key = line.substr(0, cpos);
        std::string val = line.substr(cpos + 1);
        // trim spaces
        auto trim = [](std::string& s) {
            while (!s.empty() && s[0] == ' ') s.erase(0, 1);
            while (!s.empty() && s.back() == ' ') s.pop_back();
        };
        trim(key); trim(val);

        if (key == "map") {
            state.map_name = val;
        } else if (key == "hostname") {
            state.server_hostname = val;
        } else if (key == "players") {
            // Format: "10 (20 max)" or "5 humans, 5 bots (20 max)"
            auto sp = val.find('(');
            if (sp != std::string::npos) {
                state.player_count = std::stoi(val.substr(0, sp));
            } else {
                try { state.player_count = std::stoi(val); } catch(...) {}
            }
        }
    }
}

// ── Setting renderers ────────────────────────────────────────────────────────

static void render_setting(AppState& state, const CvarSetting& s) {
    ImGui::PushID(s.cvar.c_str());

    float label_width = 180.f;
    float btn_w = 70.f;
    float input_w = 100.f;
    float avail = ImGui::GetContentRegionAvail().x - label_width;

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(s.label.c_str());
    if (!s.desc.empty()) {
        ImGui::SameLine();
        HelpMarker(s.desc.c_str());
    }

    ImGui::SameLine(label_width);
    bool auth = state.rcon && state.rcon->is_authenticated();

    switch (s.type) {

    // ── Bool: toggle buttons ──
    case SettingType::Bool: {
        ImGui::BeginDisabled(!auth);
        float btn_w2 = (avail - 4) / 2;
        if (btn_w2 > 120.f) btn_w2 = 120.f;

        ImGui::PushStyleColor(ImGuiCol_Button, Theme::bg_card);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::success);
        if (ImGui::Button("✓ 开启", ImVec2(btn_w2, 28))) {
            SendCmd(state, s.cvar + " 1");
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine(0, 4);

        ImGui::PushStyleColor(ImGuiCol_Button, Theme::bg_card);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::danger);
        if (ImGui::Button("✕ 关闭", ImVec2(btn_w2, 28))) {
            SendCmd(state, s.cvar + " 0");
        }
        ImGui::PopStyleColor(2);
        ImGui::EndDisabled();
        break;
    }

    // ── Int: input + apply button ──
    case SettingType::Int: {
        ImGui::BeginDisabled(!auth);
        auto& buf = state.str_buffers[s.cvar];
        if (buf.empty()) {
            buf = std::to_string((int)s.default_val);
        }
        ImGui::SetNextItemWidth(input_w);
        char val_buf[32] = {};
        std::strncpy(val_buf, buf.c_str(), sizeof(val_buf) - 1);
        ImGui::InputText("##val", val_buf, sizeof(val_buf),
                         ImGuiInputTextFlags_CharsDecimal);
        buf = val_buf;

        ImGui::SameLine(0, 4);
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::accent_hov);
        if (ImGui::Button("应用", ImVec2(btn_w, 28)) && !buf.empty()) {
            SendCmd(state, s.cvar + " " + buf);
            add_console_line(state, "→ 已发送: " + s.cvar + " " + buf);
        }
        ImGui::PopStyleColor(2);
        ImGui::EndDisabled();
        break;
    }

    // ── Float: input + apply button ──
    case SettingType::Float: {
        ImGui::BeginDisabled(!auth);
        auto& buf = state.str_buffers[s.cvar];
        if (buf.empty()) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << s.default_val;
            buf = oss.str();
        }
        ImGui::SetNextItemWidth(input_w);
        char val_buf[32] = {};
        std::strncpy(val_buf, buf.c_str(), sizeof(val_buf) - 1);
        ImGui::InputText("##val", val_buf, sizeof(val_buf),
                         ImGuiInputTextFlags_CharsDecimal);
        buf = val_buf;

        ImGui::SameLine(0, 4);
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::accent_hov);
        if (ImGui::Button("应用", ImVec2(btn_w, 28)) && !buf.empty()) {
            std::string cmd = s.cvar + " " + buf;
            SendCmd(state, cmd);
            add_console_line(state, "→ 已发送: " + cmd);
        }
        ImGui::PopStyleColor(2);
        ImGui::EndDisabled();
        break;
    }

    // ── String: input + apply button ──
    case SettingType::String: {
        ImGui::BeginDisabled(!auth);
        auto& buf = state.str_buffers[s.cvar];
        if (buf.empty() && !s.default_str.empty()) buf = s.default_str;
        ImGui::SetNextItemWidth(avail - btn_w - 4);
        char val_buf[512] = {};
        std::strncpy(val_buf, buf.c_str(), sizeof(val_buf) - 1);
        ImGui::InputTextWithHint("##str", "输入值...", val_buf, sizeof(val_buf));
        buf = val_buf;

        ImGui::SameLine(0, 4);
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::accent_hov);
        if (ImGui::Button("应用", ImVec2(btn_w, 28)) && !buf.empty()) {
            std::string cmd = s.cvar + " \"" + buf + "\"";
            SendCmd(state, cmd);
            add_console_line(state, "→ 已发送: " + cmd);
        }
        ImGui::PopStyleColor(2);
        ImGui::EndDisabled();
        break;
    }

    // ── Action: blue button (clear action like "结束热身") ──
    case SettingType::Action: {
        ImGui::BeginDisabled(!auth);
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::accent_hov);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::accent_hov);
        if (ImGui::Button(s.label.c_str(), ImVec2(avail, 30))) {
            // Send the cvar directly (it's a complete command)
            SendCmd(state, s.cvar);
        }
        ImGui::PopStyleColor(3);
        ImGui::EndDisabled();
        break;
    }

    // ── Combo: dropdown (selection is explicit) ──
    case SettingType::Combo: {
        int current = (int)s.default_val;
        ImGui::BeginDisabled(!auth);
        ImGui::SetNextItemWidth(avail);
        std::string preview = current >= 0 && current < (int)s.options.size()
                              ? s.options[current] : "";
        if (ImGui::BeginCombo("##combo", preview.c_str())) {
            for (int i = 0; i < (int)s.options.size(); ++i) {
                bool selected = (i == current);
                if (ImGui::Selectable(s.options[i].c_str(), &selected)) {
                    SendCmd(state, s.cvar + " " + std::to_string(i));
                    add_console_line(state,
                        "→ 已发送: " + s.cvar + " " + std::to_string(i));
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::EndDisabled();
        break;
    }
    }

    ImGui::PopID();
}

// ── Tab: Connection ──────────────────────────────────────────────────────────

static void render_connection_panel(AppState& state) {
    float avail_w = ImGui::GetContentRegionAvail().x;

    // Centered card
    float card_w = 440.f;
    float card_h = 320.f;
    ImGui::SetCursorPosX((avail_w - card_w) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 40.f);

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_med);
    ImGui::BeginChild("ConnCard", ImVec2(card_w, card_h), true);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);

    // Title
    ImGui::SetCursorPosY(24);
    ImGui::SetCursorPosX(24);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[state.font_large]);
    ImGui::TextColored(Theme::accent, "CS2 RCON 服务器管理面板");
    ImGui::PopFont();
    ImGui::SetCursorPosX(24);
    ImGui::TextDisabled("连接到你的 Counter-Strike 2 服务器");

    ImGui::Dummy(ImVec2(0, 20));

    // Server address (always tracks full user input, parses host:port in real time)
    ImGui::SetCursorPosX(24);
    ImGui::TextUnformatted("服务器地址");
    ImGui::SetCursorPosX(24);
    ImGui::SetNextItemWidth(card_w - 68);
    char addr_buf[256] = {};
    std::strncpy(addr_buf, state.conn_addr.c_str(), sizeof(addr_buf) - 1);
    ImGui::InputTextWithHint("##addr", "127.0.0.1:27015", addr_buf, sizeof(addr_buf));
    // Always re-parse the address (not just on Enter)
    state.conn_addr = addr_buf;

    // History dropdown button
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, Theme::bg_card);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::bg_hover);
    if (ImGui::BeginCombo("##hist", "▼", ImGuiComboFlags_NoPreview)) {
        if (state.server_history.empty()) {
            ImGui::TextDisabled("  暂无记录");
        }
        for (int i = (int)state.server_history.size() - 1; i >= 0; --i) {
            bool selected = (state.server_history[i] == state.conn_addr);
            if (ImGui::Selectable(state.server_history[i].c_str(), selected)) {
                state.conn_addr = state.server_history[i];
                // Restore saved password for this address
                auto pw = state.server_passwords.find(state.conn_addr);
                if (pw != state.server_passwords.end())
                    state.conn_pass = pw->second;
                // Re-parse from selection
                auto sc = state.conn_addr.find(':');
                if (sc != std::string::npos) {
                    state.conn_ip = state.conn_addr.substr(0, sc);
                    std::string sp = state.conn_addr.substr(sc + 1);
                    if (!sp.empty()) {
                        try { int p = std::stoi(sp);
                              if (p > 0 && p <= 65535) state.conn_port = p; }
                        catch (...) {}
                    }
                } else {
                    state.conn_ip = state.conn_addr;
                }
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("选择之前连接过的服务器");
    ImGui::PopStyleColor(2);
    auto colon = state.conn_addr.find(':');
    if (colon != std::string::npos) {
        state.conn_ip   = state.conn_addr.substr(0, colon);
        std::string port_str = state.conn_addr.substr(colon + 1);
        if (!port_str.empty()) {
            try {
                int p = std::stoi(port_str);
                if (p > 0 && p <= 65535) state.conn_port = p;
            } catch (...) {
                // Invalid port number — ignore
            }
        }
    } else {
        state.conn_ip   = state.conn_addr;
    }

    ImGui::Dummy(ImVec2(0, 8));

    // Password
    ImGui::SetCursorPosX(24);
    ImGui::TextUnformatted("RCON 密码");
    ImGui::SetCursorPosX(24);
    ImGui::SetNextItemWidth(card_w - 48);
    char pass_buf[128] = {};
    std::strncpy(pass_buf, state.conn_pass.c_str(), sizeof(pass_buf) - 1);
    if (ImGui::InputTextWithHint("##pass", "输入 RCON 密码",
                                  pass_buf, sizeof(pass_buf),
                                  ImGuiInputTextFlags_Password |
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
        state.conn_pass = pass_buf;
        if (state.connecting == false)
            state.connecting = true;
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        state.conn_pass = pass_buf;
    }

    ImGui::Dummy(ImVec2(0, 16));

    // Connect button
    ImGui::SetCursorPosX(24);
    bool is_conn = state.rcon && state.rcon->is_connected();
    bool is_auth = state.rcon && state.rcon->is_authenticated();

    if (is_auth) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::success);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::danger);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::danger);
        if (ImGui::Button("✓  已连接 — 断开连接",
                          ImVec2(card_w - 48, 42))) {
            state.rcon->disconnect();
        }
        ImGui::PopStyleColor(3);
    } else if (is_conn) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::warning);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::warning);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::warning);
        ImGui::Button("⟳  正在验证身份...", ImVec2(card_w - 48, 42));
        ImGui::PopStyleColor(3);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::accent_hov);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::accent_hov);
        if (ImGui::Button("🔌  连接到服务器",
                          ImVec2(card_w - 48, 42)) || state.connecting) {
            state.connecting = false;
            if (state.rcon) {
                state.conn_error.clear();
                bool ok = state.rcon->connect(state.conn_ip,
                                              (uint16_t)state.conn_port,
                                              state.conn_pass);
                if (!ok) {
                    state.conn_error = "连接失败：无法连接到服务器，请检查地址和端口";
                    add_console_line(state, "[错误] " + state.conn_error);
                } else {
                    add_console_line(state, "正在连接到 " +
                                     state.conn_ip + ":" +
                                     std::to_string(state.conn_port) + "...");
                }
            }
        }
        ImGui::PopStyleColor(3);
    }

    // Status indicator + error
    if (is_auth) {
        ImGui::SetCursorPosX(24);
        ImGui::TextColored(Theme::success, "● 已认证");
        ImGui::SameLine();
        ImGui::TextDisabled(" | %s:%d",
            state.conn_ip.c_str(), state.conn_port);
    } else if (is_conn) {
        ImGui::SetCursorPosX(24);
        ImGui::TextColored(Theme::warning, "● 连接中...");
    } else {
        ImGui::SetCursorPosX(24);
        ImGui::TextColored(Theme::text_dim, "○ 未连接");
    }

    // Show error message if any
    if (!state.conn_error.empty() && !is_auth) {
        ImGui::SetCursorPosX(24);
        ImGui::TextColored(Theme::danger, "⚠ %s", state.conn_error.c_str());
    }

    ImGui::PopStyleVar(); // FrameRounding
    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg
    ImGui::PopStyleVar(); // ChildRounding
}

// ── Tab: Dashboard ───────────────────────────────────────────────────────────

static void render_dashboard(AppState& state) {
    float avail_w = ImGui::GetContentRegionAvail().x;

    // ── Quick status cards ──
    const int card_count = 4;
    float gap = 10.f;
    float card_w = (avail_w - gap * (card_count - 1)) / card_count;
    if (card_w < 150.f) card_w = 150.f;

    auto card = [&](const char* label, const char* value,
                    const ImVec4& color, const char* icon) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_light);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
        ImGui::BeginChild(label, ImVec2(card_w, 80.f), true);
        ImGui::SetCursorPos(ImVec2(14, 12));
        ImGui::TextColored(color, "%s  %s", icon, label);
        ImGui::SetCursorPos(ImVec2(14, 40));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[state.font_large]);
        ImGui::TextColored(Theme::text_bright, "%s", value);
        ImGui::PopFont();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        if (ImGui::GetCursorPosX() < avail_w - card_w - gap)
            ImGui::SameLine(0, gap);
    };

    // Compute values
    bool auth = state.rcon && state.rcon->is_authenticated();
    std::string map_str = auth && !state.map_name.empty()
                          ? state.map_name : "—";
    std::string ply_str = auth ? std::to_string(state.player_count)
                               : "—";
    std::string score_str = auth
        ? "CT " + std::to_string(state.ct_score) + " — " +
          std::to_string(state.t_score) + " T"
        : "—";
    std::string svr_str = auth ? state.rcon->server_info() : "未连接";

    card("服务器", svr_str.c_str(), Theme::accent, "🖥");
    ImGui::SameLine(0, gap);
    card("地图", map_str.c_str(), Theme::success, "🗺");
    ImGui::SameLine(0, gap);
    card("玩家", ply_str.c_str(), Theme::warning, "👤");
    ImGui::SameLine(0, gap);
    card("比分", score_str.c_str(), Theme::info, "🏆");

    ImGui::Dummy(ImVec2(0, 14));

    // ── Presets ──
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_light);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
    ImGui::BeginChild("PresetsPanel", ImVec2(avail_w, 0), true);
    ImGui::TextColored(Theme::accent, "⚡  快速预设");
    ImGui::Dummy(ImVec2(0, 6));

    auto presets = get_presets();
    float btn_w = (ImGui::GetContentRegionAvail().x - gap * (presets.size() - 1))
                  / presets.size();
    if (btn_w < 120.f) btn_w = 120.f;

    for (const auto& [name, cmds] : presets) {
        ImGui::BeginDisabled(!auth);
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::bg_card);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::accent_hov);
        if (ImGui::Button(name.c_str(), ImVec2(btn_w, 48.f))) {
            SendCmd(state, cmds);
        }
        ImGui::PopStyleColor(3);
        ImGui::EndDisabled();
        if (&name != &presets.back().first)
            ImGui::SameLine(0, gap);
    }

    ImGui::Dummy(ImVec2(0, 8));

    // ── Quick actions row ──
    ImGui::SeparatorText("快捷操作");
    ImGui::Dummy(ImVec2(0, 4));

    const char* quick_btns[] = {
        "⏩ 重启 5秒",   "🔥 结束热身",
        "⏸ 暂停",        "▶ 恢复",
        "🤖 添加 Bot",    "🗑 踢出 Bot"
    };
    const char* quick_cmds[] = {
        "mp_restartgame 5", "mp_warmup_end",
        "mp_pause",         "mp_unpause",
        "bot_add",          "bot_kick"
    };

    for (int i = 0; i < 6; ++i) {
        ImGui::BeginDisabled(!auth);
        if (ImGui::Button(quick_btns[i], ImVec2(0, 36.f))) {
            SendCmd(state, quick_cmds[i]);
        }
        ImGui::EndDisabled();
        if ((i + 1) % 3 != 0) ImGui::SameLine();
    }

    ImGui::Dummy(ImVec2(0, 8));
    ImGui::SeparatorText("聊天 & 踢人");

    // Say
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("💬  发送消息：");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(280.f);
    ImGui::BeginDisabled(!auth);
    char say_buf[256] = {};
    std::strncpy(say_buf, state.say_buffer.c_str(), sizeof(say_buf) - 1);
    if (ImGui::InputTextWithHint("##say", "向所有玩家发送消息...",
                                  say_buf, sizeof(say_buf),
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
        state.say_buffer = say_buf;
        SendCmd(state, "say " + state.say_buffer);
        state.say_buffer.clear();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!auth || state.say_buffer.empty());
    if (ImGui::Button("发送")) {
        SendCmd(state, "say " + state.say_buffer);
        state.say_buffer.clear();
    }
    ImGui::EndDisabled();

    // Kick
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("👢  踢出玩家：");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(280.f);
    ImGui::BeginDisabled(!auth);
    char kick_buf[128] = {};
    std::strncpy(kick_buf, state.kick_buffer.c_str(), sizeof(kick_buf) - 1);
    if (ImGui::InputTextWithHint("##kick", "输入玩家名称...",
                                  kick_buf, sizeof(kick_buf),
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
        state.kick_buffer = kick_buf;
        SendCmd(state, "kick " + state.kick_buffer);
        state.kick_buffer.clear();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!auth || state.kick_buffer.empty());
    if (ImGui::Button("踢出")) {
        SendCmd(state, "kick " + state.kick_buffer);
        state.kick_buffer.clear();
    }
    ImGui::EndDisabled();

    // ── Custom presets ──
    bool auth_dash = state.rcon && state.rcon->is_authenticated();
    ImGui::Dummy(ImVec2(0, 8));
    ImGui::SeparatorText("自定义预设");
    ImGui::Dummy(ImVec2(0, 4));

    // Save current config as preset
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("预设名称：");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.f);
    char preset_buf[128] = {};
    std::strncpy(preset_buf, state.new_preset_name.c_str(), sizeof(preset_buf) - 1);
    if (ImGui::InputTextWithHint("##preset_name", "例如：我的竞技配置",
                                  preset_buf, sizeof(preset_buf))) {
        state.new_preset_name = preset_buf;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!auth_dash || state.new_preset_name.empty());
    if (ImGui::Button("💾 保存当前配置")) {
        // Build preset commands from all settings
        std::string cfg_content = "// " + state.new_preset_name + "\n";
        cfg_content += "// Saved at " + std::string(__DATE__) + " " + __TIME__ + "\n\n";
        for (const auto& g : get_all_settings()) {
            cfg_content += "// " + g.name + "\n";
            for (const auto& s : g.items) {
                if (s.type == SettingType::Action || s.type == SettingType::String)
                    continue;
                auto it = state.str_buffers.find(s.cvar);
                std::string val = (it != state.str_buffers.end() && !it->second.empty())
                                  ? it->second : std::to_string((int)s.default_val);
                cfg_content += s.cvar + " " + val + "\n";
            }
            cfg_content += "\n";
        }
        // Write to file
        std::string fname = state.new_preset_name + ".cfg";
        // Remove illegal chars
        for (auto& c : fname) {
            if (c == '\\' || c == '/' || c == ':' || c == '*' ||
                c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
                c = '_';
        }
        std::ofstream pf(fname);
        if (pf.is_open()) {
            pf << cfg_content;
            pf.close();
            add_console_line(state, "✓ 已保存预设: " + fname);
            // Refresh preset list
            state.new_preset_name.clear();
        } else {
            add_console_line(state, "✗ 保存预设失败: " + fname);
        }
        // Refresh preset list
        state.saved_presets.clear();
    }
    ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(0, 4));

    // List saved presets
    // Scan for .cfg files in current directory
    {
#ifdef _WIN32
        WIN32_FIND_DATAA ffd;
        HANDLE hf = FindFirstFileA("*.cfg", &ffd);
        if (hf != INVALID_HANDLE_VALUE) {
            state.saved_presets.clear();
            do {
                std::string fname = ffd.cFileName;
                // Skip common engine configs
                if (fname != "server.cfg" && fname != "autoexec.cfg" &&
                    fname.find("gamemode_") == std::string::npos &&
                    fname != "valve.rc") {
                    state.saved_presets.push_back(fname);
                }
            } while (FindNextFileA(hf, &ffd) != 0);
            FindClose(hf);
        }
#endif
    }

    if (state.saved_presets.empty()) {
        ImGui::TextDisabled("暂无自定义预设。输入名称后点击「保存当前配置」创建。");
    } else {
        ImGui::Text("已保存的配置：");
        ImGui::Dummy(ImVec2(0, 2));
        for (const auto& f : state.saved_presets) {
            ImGui::PushID(f.c_str());
            ImGui::TextUnformatted(f.c_str());
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 160);
            ImGui::BeginDisabled(!auth_dash);
            if (ImGui::Button("加载", ImVec2(70, 22))) {
                SendCmd(state, "exec " + f);
                add_console_line(state, "→ 执行配置: exec " + f);
            }
            ImGui::EndDisabled();
            ImGui::PopID();
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ── Tab: Settings ────────────────────────────────────────────────────────────

static void render_settings(AppState& state) {
    auto all_groups = get_all_settings();
    bool auth = state.rcon && state.rcon->is_authenticated();

    float avail_w = ImGui::GetContentRegionAvail().x;

    // Use columns to layout groups side by side
    ImGui::BeginDisabled(!auth);

    for (auto& group : all_groups) {
        // Skip quick actions from this tab - they're on dashboard
        if (group.name == "快捷操作") continue;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_light);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
        ImGui::BeginChild(group.name.c_str(),
                          ImVec2(avail_w, 0), true);

        // Group header
        ImGui::TextColored(Theme::accent, "%s  %s",
                           group.icon.c_str(), group.name.c_str());
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4));

        // Render each setting in the group
        for (auto& setting : group.items) {
            render_setting(state, setting);
            ImGui::Dummy(ImVec2(0, 2));
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 8));
    }

    ImGui::EndDisabled();
}

// ── Tab: Console ─────────────────────────────────────────────────────────────

static void render_console(AppState& state) {
    bool auth = state.rcon && state.rcon->is_authenticated();

    // Input buffer — defined early so suggestions can reference it
    char console_buf[512] = {};
    std::strncpy(console_buf, state.console_input.c_str(), sizeof(console_buf) - 1);

    // Output area
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_dark);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
    ImGui::BeginChild("ConsoleOut", ImVec2(0, -50.f), true);

    bool has_filter = !state.console_filter.empty();
    if (state.console_lines.empty()) {
        ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.4f);
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.3f);
        ImGui::TextDisabled("控制台输出将显示在此处...");
    } else {
        for (const auto& line : state.console_lines) {
            // Skip if filter active and no match
            if (has_filter && line.find(state.console_filter) == std::string::npos)
                continue;
            // Color-code lines
            if (line.find("[RCON]") != std::string::npos) {
                ImGui::TextColored(Theme::accent, "%s", line.c_str());
            } else if (line.find("> ") == 0) {
                ImGui::TextColored(Theme::text_bright, "%s", line.c_str());
            } else if (line.find("Error") != std::string::npos ||
                       line.find("FAILED") != std::string::npos ||
                       line.find("Failed") != std::string::npos) {
                ImGui::TextColored(Theme::danger, "%s", line.c_str());
            } else if (line.find("Authenticated") != std::string::npos) {
                ImGui::TextColored(Theme::success, "%s", line.c_str());
            } else {
                ImGui::TextUnformatted(line.c_str());
            }
        }

        // Auto-scroll
        if (state.scroll_console) {
            ImGui::SetScrollHereY(1.0f);
            state.scroll_console = false;
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    // Filter row
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Theme::bg_card);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80.f);
    char filter_buf[128] = {};
    std::strncpy(filter_buf, state.console_filter.c_str(), sizeof(filter_buf) - 1);
    if (ImGui::InputTextWithHint("##filter", "🔍 过滤日志...", filter_buf,
                                  sizeof(filter_buf))) {
        state.console_filter = filter_buf;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::Button("清除过滤", ImVec2(75, 0))) {
        state.console_filter.clear();
    }
    ImGui::SameLine();
    // Show filter match count
    if (!state.console_filter.empty() && !state.console_lines.empty()) {
        int match = 0;
        for (const auto& l : state.console_lines) {
            if (l.find(state.console_filter) != std::string::npos) match++;
        }
        ImGui::TextDisabled("匹配 %d 行", match);
    }

    // Input area
    ImGui::BeginDisabled(!auth);
    // ── Console input with command history ──
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Theme::bg_med);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80.f);

    // Handle up/down arrow for command history
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && !state.cmd_history.empty()) {
            if (state.cmd_history_pos < (int)state.cmd_history.size() - 1) {
                state.cmd_history_pos++;
                state.console_input = state.cmd_history[
                    state.cmd_history.size() - 1 - state.cmd_history_pos];
                std::strncpy(console_buf, state.console_input.c_str(),
                             sizeof(console_buf) - 1);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            if (state.cmd_history_pos > 0) {
                state.cmd_history_pos--;
                state.console_input = state.cmd_history[
                    state.cmd_history.size() - 1 - state.cmd_history_pos];
            } else {
                state.cmd_history_pos = -1;
                state.console_input.clear();
            }
            std::strncpy(console_buf, state.console_input.c_str(),
                         sizeof(console_buf) - 1);
        }
    }

    bool input_active = ImGui::InputTextWithHint("##console_input", "输入命令...",
                                  console_buf, sizeof(console_buf),
                                  ImGuiInputTextFlags_EnterReturnsTrue);
    // Sync the typed text back every frame so suggestions can use it
    state.console_input = console_buf;

    // Handle Enter on console input
    if (input_active) {
        state.console_input = console_buf;
        if (!state.console_input.empty()) {
            SendCmd(state, state.console_input);
            if (state.cmd_history.empty() ||
                state.cmd_history.back() != state.console_input) {
                state.cmd_history.push_back(state.console_input);
            }
            state.cmd_history_pos = -1;
            state.console_input.clear();
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, Theme::accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::accent_hov);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::accent_hov);
    if (ImGui::Button("发送", ImVec2(70, 0))) {
        if (!state.console_input.empty()) {
            SendCmd(state, state.console_input);
            if (state.cmd_history.empty() ||
                state.cmd_history.back() != state.console_input) {
                state.cmd_history.push_back(state.console_input);
            }
            state.cmd_history_pos = -1;
            state.console_input.clear();
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("清空", ImVec2(60, 0))) {
        state.console_lines.clear();
    }
}

// ── Tab: Players ─────────────────────────────────────────────────────────────

static void render_players(AppState& state) {
    bool auth = state.rcon && state.rcon->is_authenticated();

    ImGui::BeginDisabled(!auth);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_light);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
    ImGui::BeginChild("PlayersPanel", ImVec2(0, 0), true);

    // Header
    ImGui::TextColored(Theme::accent, "👤  玩家列表");
    ImGui::SameLine();
    if (ImGui::SmallButton("⟳ 刷新")) {
        SendCmd(state, "status");
        SendCmd(state, "stats");
    }
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4));

    // Placeholder player list
    if (state.players.empty()) {
        ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.3f);
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.2f);
        ImGui::TextDisabled("点击「刷新」或运行 'status' 查看玩家");
    } else {
        if (ImGui::BeginTable("PlayerTable", 6,
                              ImGuiTableFlags_Resizable |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_BordersV |
                              ImGuiTableFlags_ScrollY,
                              ImVec2(0, ImGui::GetContentRegionAvail().y - 50))) {
            ImGui::TableSetupColumn("玩家名称", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("分数",    ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("击杀",   ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("死亡",   ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("延迟",   ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("操作",   ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableHeadersRow();

            for (auto& p : state.players) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(p.name.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%d", p.score);
                ImGui::TableNextColumn(); ImGui::Text("%d", p.kills);
                ImGui::TableNextColumn(); ImGui::Text("%d", p.deaths);
                ImGui::TableNextColumn(); ImGui::Text("%d", p.ping);

                ImGui::TableNextColumn();
                ImGui::PushID(p.name.c_str());
                if (ImGui::SmallButton("踢出")) {
                    SendCmd(state, "kick \"" + p.name + "\"");
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("封禁")) {
                    SendCmd(state, "ban \"" + p.name + "\"");
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    // Send message to all
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::SeparatorText("发送广播消息");
    ImGui::Dummy(ImVec2(0, 4));

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80);
    char say_buf[256] = {};
    std::strncpy(say_buf, state.say_buffer.c_str(), sizeof(say_buf) - 1);
    if (ImGui::InputTextWithHint("##say_players", "输入消息...",
                                  say_buf, sizeof(say_buf),
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
        state.say_buffer = say_buf;
        SendCmd(state, "say " + state.say_buffer);
        state.say_buffer.clear();
    }

    ImGui::SameLine();
    if (ImGui::Button("发送全部")) {
        if (!state.say_buffer.empty()) {
            SendCmd(state, "say " + state.say_buffer);
            state.say_buffer.clear();
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::EndDisabled();
}

// ── Tab: Maps ────────────────────────────────────────────────────────────────

static void render_maps(AppState& state) {
    bool auth = state.rcon && state.rcon->is_authenticated();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_light);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
    ImGui::BeginChild("MapsPanel", ImVec2(0, 0), true);

    ImGui::TextColored(Theme::accent, "🗺  地图管理");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 8));

    // Current map
    ImGui::TextUnformatted("当前地图：");
    ImGui::SameLine();
    ImGui::TextColored(Theme::text_bright, "%s",
        auth && !state.map_name.empty() ? state.map_name.c_str() : "—");

    ImGui::Dummy(ImVec2(0, 16));

    // Map change
    ImGui::TextUnformatted("切换地图（重启）：");
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::BeginDisabled(!auth);
    char map_buf[128] = {};
    std::strncpy(map_buf, "de_dust2", sizeof(map_buf) - 1);
    ImGui::SetNextItemWidth(300.f);
    if (ImGui::InputTextWithHint("##map_name", "输入地图名...",
                                  map_buf, sizeof(map_buf),
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
        SendCmd(state, std::string("map ") + map_buf);
    }
    ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(0, 4));

    // Common maps quick buttons
    ImGui::TextUnformatted("快速选择：");
    ImGui::Dummy(ImVec2(0, 4));

    const char* common_maps[] = {
        "de_dust2", "de_mirage", "de_inferno", "de_nuke",
        "de_overpass", "de_ancient", "de_vertigo", "de_anubis",
        "cs_office", "cs_italy", "de_lake", "de_shortdust"
    };

    float btn_w = (ImGui::GetContentRegionAvail().x - 20.f) / 4.f;
    if (btn_w > 160.f) btn_w = 160.f;

    for (int i = 0; i < 12; ++i) {
        ImGui::BeginDisabled(!auth);
        if (ImGui::Button(common_maps[i], ImVec2(btn_w, 32.f))) {
            SendCmd(state, std::string("map ") + common_maps[i]);
        }
        ImGui::EndDisabled();
        if ((i + 1) % 4 != 0) ImGui::SameLine();
    }

    ImGui::Dummy(ImVec2(0, 12));

    // Workshop map
    ImGui::SeparatorText("工坊地图");
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::BeginDisabled(!auth);
    char ws_buf[256] = {};
    ImGui::SetNextItemWidth(400.f);
    if (ImGui::InputTextWithHint("##workshop", "输入工坊地图 ID 或 URL...",
                                  ws_buf, sizeof(ws_buf),
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
        SendCmd(state, std::string("ds_workshop_changelevel ") + ws_buf);
    }
    ImGui::EndDisabled();

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ── Sidebar ──────────────────────────────────────────────────────────────────

static void render_sidebar(AppState& state) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_med);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
    ImGui::BeginChild("Sidebar", ImVec2(180, 0), true);
    ImGui::Dummy(ImVec2(0, 4));

    struct TabItem {
        const char* icon;
        const char* label;
    };

    TabItem tabs[] = {
        {"🔌", "连接"},
        {"📊", "仪表盘"},
        {"⚙",  "设置"},
        {"👤", "玩家"},
        {"🗺", "地图"},
        {"📟", "控制台"},
    };

    bool auth = state.rcon && state.rcon->is_authenticated();

    for (int i = 0; i < IM_ARRAYSIZE(tabs); ++i) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
        ImGui::PushStyleColor(ImGuiCol_Button,
            (i == state.active_tab) ? Theme::bg_active : ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            (i == state.active_tab) ? Theme::bg_active : Theme::bg_hover);

        // Disable some tabs when not connected
        bool tab_disabled = (i > 0 && !auth);
        ImGui::BeginDisabled(tab_disabled);

        float btn_w = ImGui::GetContentRegionAvail().x - 4.f;
        if (ImGui::Button((std::string(tabs[i].icon) + "  " + tabs[i].label).c_str(),
                          ImVec2(btn_w, 40.f))) {
            state.active_tab = i;
        }

        ImGui::EndDisabled();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        ImGui::Dummy(ImVec2(0, 2));
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ── Main UI entry point ─────────────────────────────────────────────────────

void render_ui(AppState& state) {
    // ── Process RCON responses ──
    if (state.rcon) {
        auto responses = state.rcon->consume_responses();
        for (const auto& resp : responses) {
            add_console_line(state, resp);
            // Try to parse status output for map/player info
            if (resp.find("map") != std::string::npos &&
                resp.find("hostname") != std::string::npos) {
                parse_server_status(state, resp);
            }
        }

        // Check for auth state changes
        static bool was_auth = false;
        bool is_auth = state.rcon->is_authenticated();
        if (is_auth && !was_auth) {
            add_console_line(state,
                "✓ 已连接并认证到 " + state.rcon->server_info());
            // Query server info
            state.rcon->send_command("status");
            // Save to server history
            add_history(state, state.conn_addr);
            was_auth = true;
        }
        if (!is_auth && was_auth) {
            add_console_line(state, "✗ 已从服务器断开连接");
            was_auth = false;
        }
    }

    // ── Main layout ──
    ImGui::Dummy(ImVec2(0, 2));

    ImGui::BeginDisabled(state.rcon && state.rcon->is_connected() && !state.rcon->is_authenticated());
    // Style the main content area
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::bg_dark);

    if (state.rcon && state.rcon->is_authenticated()) {
        // ── Connected: sidebar + content + console ──
        render_sidebar(state);

        ImGui::SameLine();

        // Main content area
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.f);
        ImGui::BeginChild("MainContent", ImVec2(0, 0), false);
        ImGui::Dummy(ImVec2(0, 4));

        switch (state.active_tab) {
            case 0: render_connection_panel(state); break;
            case 1: render_dashboard(state); break;
            case 2: render_settings(state); break;
            case 3: render_players(state); break;
            case 4: render_maps(state); break;
            case 5: render_console(state); break;
            default: render_dashboard(state); break;
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();

    } else {
        // ── Not connected: show connection panel ──
        render_connection_panel(state);
    }

    ImGui::PopStyleColor(); // ChildBg
    ImGui::EndDisabled();
}
