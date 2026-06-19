#include "imgui.h"
#include "theme.h"

void apply_custom_theme() {
    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    // ── Style ────────────────────────────────────────────────────────────────
    style.WindowPadding       = ImVec2(12, 12);
    style.FramePadding        = ImVec2(10, 6);
    style.ItemSpacing         = ImVec2(8, 6);
    style.ItemInnerSpacing    = ImVec2(6, 4);
    style.TouchExtraPadding   = ImVec2(0, 0);
    style.IndentSpacing       = 20.f;
    style.ScrollbarSize       = 12.f;
    style.GrabMinSize         = 10.f;

    style.WindowBorderSize    = 1.f;
    style.ChildBorderSize     = 1.f;
    style.PopupBorderSize     = 1.f;
    style.FrameBorderSize     = 0.f;
    style.TabBorderSize       = 0.f;
    style.TabBarBorderSize    = 0.f;

    style.WindowRounding      = 8.f;
    style.ChildRounding       = 6.f;
    style.FrameRounding       = 4.f;
    style.PopupRounding       = 6.f;
    style.ScrollbarRounding   = 8.f;
    style.GrabRounding        = 4.f;
    style.TabRounding         = 4.f;

    style.WindowTitleAlign    = ImVec2(0.5f, 0.5f);
    style.ButtonTextAlign     = ImVec2(0.5f, 0.5f);

    style.AntiAliasedLines    = true;
    style.AntiAliasedFill     = true;

    style.WindowMinSize       = ImVec2(1024, 640);

    // ── Colors ───────────────────────────────────────────────────────────────
    using namespace Theme;

    // Window / panel fills
    colors[ImGuiCol_WindowBg]           = bg_dark;
    colors[ImGuiCol_ChildBg]            = bg_med;
    colors[ImGuiCol_PopupBg]            = bg_light;
    colors[ImGuiCol_Border]             = border;
    colors[ImGuiCol_BorderShadow]       = ImVec4(0, 0, 0, 0);

    // Frame (widget background)
    colors[ImGuiCol_FrameBg]            = bg_card;
    colors[ImGuiCol_FrameBgHovered]     = bg_hover;
    colors[ImGuiCol_FrameBgActive]      = bg_active;

    // Title
    colors[ImGuiCol_TitleBg]            = title_bg;
    colors[ImGuiCol_TitleBgActive]      = title_bg;
    colors[ImGuiCol_TitleBgCollapsed]   = title_bg;

    // Menu bar
    colors[ImGuiCol_MenuBarBg]          = bg_med;

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]        = bg_dark;
    colors[ImGuiCol_ScrollbarGrab]      = bg_hover;
    colors[ImGuiCol_ScrollbarGrabHovered] = bg_active;
    colors[ImGuiCol_ScrollbarGrabActive]  = accent;

    // Checkbox / radio
    colors[ImGuiCol_CheckMark]          = accent;

    // Slider
    colors[ImGuiCol_SliderGrab]         = accent;
    colors[ImGuiCol_SliderGrabActive]   = accent_hov;

    // Button
    colors[ImGuiCol_Button]             = bg_card;
    colors[ImGuiCol_ButtonHovered]      = accent;
    colors[ImGuiCol_ButtonActive]       = accent_hov;

    // Header (collapsing header, tree node, etc.)
    colors[ImGuiCol_Header]             = ImVec4(0.18f, 0.20f, 0.27f, 0.80f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.22f, 0.25f, 0.33f, 0.80f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.26f, 0.29f, 0.38f, 0.80f);

    // Tabs
    colors[ImGuiCol_Tab]                = bg_med;
    colors[ImGuiCol_TabHovered]         = accent;
    colors[ImGuiCol_TabActive]          = bg_light;
    colors[ImGuiCol_TabUnfocused]       = bg_med;
    colors[ImGuiCol_TabUnfocusedActive] = bg_light;
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0,0,0,0);

    // Separator
    colors[ImGuiCol_Separator]          = border;
    colors[ImGuiCol_SeparatorHovered]   = border_lt;
    colors[ImGuiCol_SeparatorActive]    = accent;

    // Resize grip
    colors[ImGuiCol_ResizeGrip]         = ImVec4(0.20f, 0.22f, 0.30f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.25f, 0.28f, 0.38f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]   = accent;

    // Text
    colors[ImGuiCol_Text]               = text;
    colors[ImGuiCol_TextDisabled]       = text_dim;
    colors[ImGuiCol_TextSelectedBg]     = ImVec4(accent.x, accent.y, accent.z, 0.25f);

    // Highlight / nav
    colors[ImGuiCol_NavHighlight]       = accent;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1,1,1,0.70f);
    colors[ImGuiCol_NavWindowingDimBg]  = ImVec4(0,0,0,0.20f);

    // Modal dim
    colors[ImGuiCol_ModalWindowDimBg]   = ImVec4(0,0,0,0.50f);

    // Drag drop
    colors[ImGuiCol_DragDropTarget]     = accent;

    // Plot
    colors[ImGuiCol_PlotLines]          = accent;
    colors[ImGuiCol_PlotLinesHovered]   = accent_hov;
    colors[ImGuiCol_PlotHistogram]      = accent;
    colors[ImGuiCol_PlotHistogramHovered] = accent_hov;

    // Tables
    colors[ImGuiCol_TableHeaderBg]      = bg_light;
    colors[ImGuiCol_TableBorderStrong]  = border_lt;
    colors[ImGuiCol_TableBorderLight]   = border;
    colors[ImGuiCol_TableRowBg]         = ImVec4(0,0,0,0);
    colors[ImGuiCol_TableRowBgAlt]      = ImVec4(1,1,1,0.03f);
}
