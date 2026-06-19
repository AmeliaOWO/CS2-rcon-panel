#pragma once
#include "imgui.h"

void apply_custom_theme();
void apply_style_vars();

// Accessible color tokens for use anywhere
namespace Theme {
    // Background layers
    inline ImVec4 bg_dark    (0.07f, 0.08f, 0.11f, 1.00f); // #12141c
    inline ImVec4 bg_med     (0.10f, 0.11f, 0.15f, 1.00f); // #1a1d26
    inline ImVec4 bg_light   (0.14f, 0.15f, 0.20f, 1.00f); // #242736
    inline ImVec4 bg_card    (0.11f, 0.12f, 0.17f, 1.00f); // #1d1f2b
    inline ImVec4 bg_hover   (0.18f, 0.19f, 0.25f, 1.00f); // #2e3140
    inline ImVec4 bg_active  (0.22f, 0.23f, 0.30f, 1.00f); // #383b4d

    // Accent colours
    inline ImVec4 accent     (0.24f, 0.52f, 0.96f, 1.00f); // #3d85f5
    inline ImVec4 accent_hov (0.30f, 0.58f, 1.00f, 1.00f); // #4d94ff
    inline ImVec4 success    (0.22f, 0.72f, 0.42f, 1.00f); // #38b86b
    inline ImVec4 warning    (0.95f, 0.65f, 0.15f, 1.00f); // #f2a626
    inline ImVec4 danger     (0.82f, 0.24f, 0.30f, 1.00f); // #d13d4d
    inline ImVec4 info       (0.30f, 0.65f, 0.85f, 1.00f); // #4da6d9

    // Text
    inline ImVec4 text       (0.90f, 0.91f, 0.93f, 1.00f); // #e6e8ed
    inline ImVec4 text_dim   (0.50f, 0.53f, 0.60f, 1.00f); // #808799
    inline ImVec4 text_bright(1.00f, 1.00f, 1.00f, 1.00f);

    // Border
    inline ImVec4 border     (0.18f, 0.20f, 0.26f, 1.00f); // #2e3442
    inline ImVec4 border_lt  (0.22f, 0.24f, 0.31f, 1.00f); // #383d4f

    // Misc
    inline ImVec4 title_bg   (0.09f, 0.10f, 0.13f, 1.00f); // #171a22
    inline ImVec4 btn_text   (0.95f, 0.96f, 0.97f, 1.00f);
}
