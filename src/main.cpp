#include "rcon_client.h"
#include "ui.h"
#include "theme.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_opengl3_loader.h"

#include <cstdio>
#include <cstdlib>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// ── Error callback ───────────────────────────────────────────────────────────
static void glfw_error_callback(int error, const char* desc) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, desc);
}

// ── WinMain wrapper (for /SUBSYSTEM:WINDOWS) ─────────────────────────────────
#ifdef _WIN32
#include <windows.h>
int main(int, char**);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main(__argc, __argv);
}
#endif

// ── Main ─────────────────────────────────────────────────────────────────────
int main(int, char**) {
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    // Decide GL+GLSL versions
#if defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#elif defined(_WIN32)
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(1400, 860,
                                          "CS2 RCON 服务器管理面板  —  v1.0",
                                          nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // ── Setup Dear ImGui ─────────────────────────────────────────────────────
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // don't save layout

    // ── Fonts — load Chinese + Emoji ──────────
    ImFont* font_default = nullptr;
    ImFont* font_title   = nullptr;

    // Emoji ranges: key UI emoji + misc symbols + geometric shapes
    static const ImWchar emoji_ranges[] = {
        0x1F300, 0x1FAFF,  // Emoji (🔌📊👤🗺📟🎯💰🔥🤖🖥🔫💬🏆 etc.)
        0x2600, 0x26FF,    // Misc symbols (⚡⚙⚠☀★ etc.)
        0x2700, 0x27BF,    // Dingbats (✉✓✗✘ etc.)
        0x2300, 0x23FF,    // Misc technical (⏩⏸⏰⌂ etc.)
        0x25A0, 0x25FF,    // Geometric shapes (●○■◆ etc.)
        0x2B00, 0x2BFF,    // Misc arrows
        0
    };

    const char* chinese_fonts[] = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/msyhbd.ttc",
    };

    for (auto path : chinese_fonts) {
        if (font_default) break;
        FILE* f = nullptr;
        if (fopen_s(&f, path, "rb") == 0 && f) {
            fclose(f);
            // Load Chinese font (16px) + merge emoji
            {
                ImFontConfig cfg;
                cfg.FontNo = 0;
                font_default = io.Fonts->AddFontFromFileTTF(path, 16.0f, &cfg,
                    io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
            }
            if (font_default && io.Fonts->Fonts.Size > 0) {
                // Merge Segoe UI Emoji for emoji glyphs
                FILE* ef = nullptr;
                if (fopen_s(&ef, "C:/Windows/Fonts/seguiemj.ttf", "rb") == 0 && ef) {
                    fclose(ef);
                    ImFontConfig cfg_merge;
                    cfg_merge.MergeMode = true;
                    cfg_merge.FontNo = 0;
                    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/seguiemj.ttf",
                        16.0f, &cfg_merge, emoji_ranges);
                }
            }
            // Large title font (22px) + emoji
            {
                ImFontConfig cfg2;
                cfg2.FontNo = 0;
                font_title = io.Fonts->AddFontFromFileTTF(path, 22.0f, &cfg2,
                    io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
            }
            if (font_title) {
                FILE* ef = nullptr;
                if (fopen_s(&ef, "C:/Windows/Fonts/seguiemj.ttf", "rb") == 0 && ef) {
                    fclose(ef);
                    ImFontConfig cfg_merge;
                    cfg_merge.MergeMode = true;
                    cfg_merge.FontNo = 0;
                    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/seguiemj.ttf",
                        22.0f, &cfg_merge, emoji_ranges);
                }
            }
        }
    }

    if (!font_default) {
        font_default = io.Fonts->AddFontDefault();
        ImFontConfig big_cfg;
        font_title = io.Fonts->AddFontDefault(&big_cfg);
    }

    // Fonts[1] = large title font (22px) if loaded
    bool has_large_font = (font_title != nullptr && font_default != font_title);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Apply custom theme
    apply_custom_theme();

    // ── App state ────────────────────────────────────────────────────────────
    AppState app_state;
    app_state.font_large = has_large_font ? 1 : 0;
    RCONClient rcon;
    app_state.rcon = &rcon;

    // Load saved server history
    load_server_history(app_state);

    // Set up auth callback
    rcon.set_on_auth([&](bool success) {
        if (success) {
            app_state.conn_error.clear();
            add_console_line(app_state, "[RCON] 认证成功！");
            // Refresh server info
            rcon.send_command("status");
        } else {
            app_state.conn_error = "认证失败：RCON 密码错误？";
            add_console_line(app_state, "[RCON] " + app_state.conn_error);
        }
    });

    rcon.set_on_disconnect([&]() {
        add_console_line(app_state, "[RCON] 连接断开 / 丢失");
    });

    // ── Main loop ────────────────────────────────────────────────────────────
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ── Auto-refresh: poll server status every 10 seconds ──
        if (rcon.is_authenticated()) {
            double now = ImGui::GetTime();
            if (now - app_state.last_refresh > 10.0) {
                app_state.last_refresh = now;
                rcon.send_command("status");
                rcon.send_command("stats");
            }
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ── Build fullscreen window as root ──
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::bg_dark);

        ImGui::Begin("RootWindow", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus |
                     ImGuiWindowFlags_NoNavFocus);

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);

        // Title bar area
        {
            float title_h = 36.f;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::title_bg);
            ImGui::BeginChild("TitleBar", ImVec2(0, title_h), false);

            ImGui::SetCursorPos(ImVec2(14, 7));
            ImGui::TextColored(Theme::accent, "CS2 RCON 服务器管理面板");

            // Server info in center of title bar
            if (rcon.is_authenticated()) {
                std::string info;
                if (!app_state.server_hostname.empty())
                    info = app_state.server_hostname + "  |  ";
                info += app_state.map_name.empty() ? "—" : app_state.map_name;
                info += "  |  ";
                info += std::to_string(app_state.player_count) + " 人";
                info += "  |  CT " + std::to_string(app_state.ct_score) +
                        " — " + std::to_string(app_state.t_score) + " T";
                ImGui::SetCursorPos(ImVec2(
                    (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(info.c_str()).x) / 2 + 14, 7));
                ImGui::TextDisabled("%s", info.c_str());
            }

            // Connection status indicator
            if (rcon.is_authenticated()) {
                ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x - 200, 7));
                ImGui::TextColored(Theme::success, "●  %s",
                                   rcon.server_info().c_str());
            } else if (rcon.is_connected()) {
                ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x - 180, 7));
                ImGui::TextColored(Theme::warning, "●  验证中...");
            } else {
                ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x - 160, 7));
                ImGui::TextColored(Theme::text_dim, "○  未连接");
            }

            // Disconnect button in top-right
            if (rcon.is_connected()) {
                ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x - 80, 4));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::danger);
                if (ImGui::Button("✕ 断开连接", ImVec2(75, 28))) {
                    rcon.disconnect();
                }
                ImGui::PopStyleColor(2);
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0, 4));
        }

        // Main UI
        render_ui(app_state);

        ImGui::End(); // RootWindow

        // ── Render ──
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(Theme::bg_dark.x, Theme::bg_dark.y,
                     Theme::bg_dark.z, Theme::bg_dark.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // ── Cleanup ──────────────────────────────────────────────────────────────
    rcon.disconnect();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
