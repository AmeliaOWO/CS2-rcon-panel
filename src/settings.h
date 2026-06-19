#pragma once
#include <string>
#include <vector>
#include <functional>

// ── 设置类型 ─────────────────────────────────────────────────────────────────
enum class SettingType {
    Bool,       // 开关
    Int,        // 整数滑块
    Float,      // 浮点滑块
    String,     // 文本输入
    Action,     // 一次性操作按钮
    Combo,      // 下拉选择
};

// ── 单个服务器设置项 ──────────────────────────────────────────────────────────
struct CvarSetting {
    std::string cvar;          // 实际的 cvar / 命令
    std::string label;         // 人类可读的名称
    std::string desc;          // 鼠标悬停提示
    SettingType type;

    // 数值范围（用于 Int / Float）
    float min_val = 0.f;
    float max_val = 0.f;
    float default_val = 0.f;

    // 默认字符串（用于 String / Combo）
    std::string default_str;

    // 下拉选项
    std::vector<std::string> options;
};

// ── 命名设置分组 ─────────────────────────────────────────────────────────────
struct SettingGroup {
    std::string name;
    std::string icon;       // Unicode 图标
    std::vector<CvarSetting> items;
};

// ── 获取所有预定义的 CS2 设置 ────────────────────────────────────────────────
inline std::vector<SettingGroup> get_all_settings() {
    using ST = SettingType;

    return {
        // ═══════════ 回合 / 比赛 ═══════════
        {"回合 / 比赛",   "🎯", {
            {"mp_roundtime",             "回合时间",               "每回合时长（分钟）",                             ST::Float, 0.f, 60.f, 5.f},
            {"mp_roundtime_defuse",      "回合时间（拆弹图）",     "拆弹地图上的回合时长",                           ST::Float, 0.f, 60.f, 5.f},
            {"mp_roundtime_hostage",     "回合时间（人质图）",     "人质地图上的回合时长",                           ST::Float, 0.f, 60.f, 5.f},
            {"mp_maxrounds",             "最大回合数",             "半场 / 比赛结束前的总回合数",                    ST::Int,   0.f, 100.f, 30.f},
            {"mp_timelimit",             "时间限制",               "地图时间限制（分钟，0=无限制）",                 ST::Int,   0.f, 120.f, 5.f},
            {"mp_freezetime",            "冻结时间",               "回合开始时的冻结时间（秒）",                    ST::Int,   0.f, 60.f, 15.f},
            {"mp_buytime",               "购买时间",               "购买武器的时间窗口（秒）",                      ST::Int,   0.f, 90.f, 20.f},
            {"mp_round_restart_delay",   "重启延迟",               "回合重启前的延迟（秒）",                        ST::Int,   0.f, 30.f, 5.f},
            {"mp_restartgame",           "⏩ 重启游戏",            "在 N 秒后重启游戏（0=取消）",                   ST::Int,   0.f, 60.f, 5.f},
        }},

        // ═══════════ 经济 ═══════════
        {"经济",         "💰", {
            {"mp_startmoney",            "起始金钱",               "每回合玩家起始金钱",                            ST::Int,   0.f, 16000.f, 800.f},
            {"mp_maxmoney",              "最大金钱",               "玩家可持有的最大金钱",                          ST::Int,   0.f, 65535.f, 16000.f},
            {"mp_playercashawards",      "玩家金钱奖励",           "启用玩家行为的金钱奖励",                        ST::Bool,  0.f, 1.f, 1.f},
            {"mp_teamcashawards",        "团队金钱奖励",           "启用团队行为的金钱奖励",                        ST::Bool,  0.f, 1.f, 1.f},
            {"mp_death_drop_gun",        "死亡掉落武器",           "0=关闭, 1=总是, 2=仅存活时",                   ST::Combo, 0.f, 0.f, 1.f, "", {"关闭", "总是掉落", "仅存活时"}},
            {"mp_defuser_allocation",    "拆弹器分配",             "0=无, 1=所有人, 2=仅反恐精英",                 ST::Combo, 0.f, 0.f, 0.f, "", {"禁用", "所有人", "仅CT"}},
        }},

        // ═══════════ 热身 ═══════════
        {"热身",          "🔥", {
            {"mp_warmuptime",            "热身时长",               "热身阶段时长（分钟）",                          ST::Int,   0.f, 120.f, 30.f},
            {"mp_warmup_start",          "▶ 开始热身",            "开始热身阶段",                                  ST::Action},
            {"mp_warmup_end",            "■ 结束热身",            "强制结束热身并开始比赛",                        ST::Action},
            {"mp_do_warmup_off",         "禁用热身",               "关闭后续比赛的热身",                            ST::Action},
            {"mp_do_warmup_on",          "启用热身",               "开启后续比赛的热身",                            ST::Action},
        }},

        // ═══════════ 加时赛 ═══════════
        {"加时赛",        "⏰", {
            {"mp_overtime_enable",       "启用加时赛",             "允许在回合达到上限后进行加时赛",                ST::Bool,  0.f, 1.f, 0.f},
            {"mp_overtime_maxrounds",    "加时赛最大回合",         "加时赛每半场的回合数",                          ST::Int,   1.f, 10.f, 6.f},
            {"mp_overtime_startmoney",   "加时赛起始金钱",         "加时赛中的起始金钱",                            ST::Int,   800.f, 16000.f, 10000.f},
            {"mp_overtime_halftime",     "加时赛半场交换",         "加时赛中启用半场交换",                          ST::Bool,  0.f, 1.f, 1.f},
        }},

        // ═══════════ 游戏规则 ═══════════
        {"游戏规则",  "⚙️", {
            {"mp_friendlyfire",          "友军伤害",               "开启对队友的伤害",                              ST::Bool,  0.f, 1.f, 0.f},
            {"mp_autoteambalance",       "自动平衡队伍",           "自动平衡双方人数",                              ST::Bool,  0.f, 1.f, 1.f},
            {"mp_limitteams",            "队伍人数差限制",         "两队最大人数差",                                ST::Int,   0.f, 10.f, 2.f},
            {"mp_buy_anywhere",          "随地购买",               "允许在地图任意位置购买武器",                    ST::Bool,  0.f, 1.f, 0.f},
            {"mp_buy_during_immunity",   "免疫期间购买",           "允许在出生免疫期间购买",                        ST::Bool,  0.f, 1.f, 0.f},
            {"sv_infinite_ammo",         "无限弹药",               "0=关闭, 1=无限弹夹, 2=无限备弹",               ST::Combo, 0.f, 0.f, 0.f, "", {"关闭", "无限弹夹", "无限备弹"}},
            {"sv_gravity",               "重力",                   "世界重力（800=正常）",                          ST::Int,   100.f, 2000.f, 800.f},
            {"sv_cheats",               "启用作弊",                "允许使用作弊指令",                              ST::Bool,  0.f, 1.f, 0.f},
            {"mp_halftime",              "半场交换",               "在最大回合数/2时交换阵营",                      ST::Bool,  0.f, 1.f, 1.f},
            {"mp_match_can_clinch",      "允许锁定胜局",           "当一方锁定胜局时比赛结束",                      ST::Bool, 0.f, 1.f, 1.f},
        }},

        // ═══════════ 武器 & 投掷物 ═══════════
        {"武器 / 投掷物", "🔫", {
            {"ammo_grenade_limit_total",         "最大投掷物数",            "玩家可携带的投掷物总数",                           ST::Int, 0.f, 5.f, 4.f},
            {"ammo_grenade_limit_flashbang",     "最大闪光弹",              "每人可携带的闪光弹数量",                           ST::Int, 0.f, 5.f, 2.f},
            {"ammo_grenade_limit_smokes",        "最大烟雾弹",              "每人可携带的烟雾弹数量",                           ST::Int, 0.f, 5.f, 1.f},
            {"ammo_grenade_limit_incendiary",    "最大燃烧弹",              "每人可携带的燃烧弹/燃烧瓶数量",                    ST::Int, 0.f, 5.f, 1.f},
            {"ammo_grenade_limit_decoy",         "最大诱饵弹",              "每人可携带的诱饵弹数量",                           ST::Int, 0.f, 5.f, 1.f},
            {"sv_grenade_trajectory",           "投掷物轨迹",              "显示投掷物轨迹线（需启用作弊）",                   ST::Bool, 0.f, 1.f, 0.f},
            {"sv_showimpacts",                  "显示弹道",                "显示子弹命中点（需启用作弊）",                      ST::Bool, 0.f, 1.f, 0.f},
        }},

        // ═══════════ 机器人 ═══════════
        {"机器人",          "🤖", {
            {"bot_add",                  "添加机器人",               "向游戏中添加机器人",                              ST::Action},
            {"bot_kick",                 "踢出所有机器人",           "移除游戏中所有机器人",                            ST::Action},
            {"bot_kill",                 "杀死所有机器人",           "杀死所有活跃的机器人",                            ST::Action},
            {"bot_difficulty",           "机器人难度",               "0=简单 1=普通 2=困难 3=专家",                     ST::Combo, 0.f, 0.f, 1.f, "", {"简单", "普通", "困难", "专家"}},
            {"bot_quota",                "机器人数量",               "维护的机器人数量",                                ST::Int,   0.f, 64.f, 10.f},
            {"bot_quota_mode",           "机器人配额模式",           "fill=填满空位, normal=固定数量",                  ST::Combo, 0.f, 0.f, 0.f, "", {"fill（填满）", "normal（固定）"}},
            {"bot_join_after_player",    "玩家加入后添加机器人",      "有玩家连接后自动添加机器人",                      ST::Bool,  0.f, 1.f, 1.f},
        }},

        // ═══════════ 服务器 ═══════════
        {"服务器",          "🖥️", {
            {"hostname",               "服务器名称",                 "设置服务器主机名",                       ST::String, 0.f, 0.f, 0.f, "CS2 服务器"},
            {"sv_password",            "RCON 密码",                 "设置服务器密码（留空则移除）",           ST::String},
            {"sv_maxplayers",          "最大玩家数",                 "最大玩家数量",                          ST::Int,   1.f, 64.f, 32.f},
            {"sv_visiblemaxplayers",   "显示最大玩家数",             "服务器浏览器中显示的最大玩家数",          ST::Int,   1.f, 64.f, 32.f},
            {"exec",                   "执行配置文件",              "执行服务器配置文件",                    ST::String, 0.f, 0.f, 0.f, ""},
            {"say",                    "发送聊天消息",               "向所有玩家发送消息",                    ST::Action},
        }},

        // ═══════════ 地图 ═══════════
        {"地图",            "🗺️", {
            {"map",                    "切换地图（重启）",           "切换地图（完全重启）",                   ST::String, 0.f, 0.f, 0.f, "de_dust2"},
            {"changelevel",            "切换地图",                   "立即切换地图",                           ST::String, 0.f, 0.f, 0.f, "de_dust2"},
            {"ds_workshop_changelevel","工坊地图",                   "通过 ID 加载工坊地图",                   ST::String, 0.f, 0.f, 0.f, ""},
        }},

        // ═══════════ 快捷操作 ═══════════
        {"快捷操作",   "⚡", {
            {"mp_pause",                 "暂停比赛",                "暂停比赛（暂停计时器）",                ST::Action},
            {"mp_unpause",              "恢复比赛",                "恢复比赛",                             ST::Action},
            {"mp_restartgame 10",       "10 秒后重启",            "在 10 秒后重启游戏",                   ST::Action},
            {"mp_restartgame 5",        "5 秒后重启",             "在 5 秒后重启游戏",                    ST::Action},
            {"mp_restartgame 1",        "立即重启",               "立即重启游戏",                         ST::Action},
            {"mp_restartgame 0",        "取消重启",               "取消待处理的重启",                     ST::Action},
            {"sv_cheats 1; sv_grenade_trajectory 1; sv_showimpacts 1",
                                        "开启练枪模式",            "启用作弊 + 投掷物轨迹 + 弹道显示",      ST::Action},
            {"kick",                   "踢出玩家",                "按名称踢出玩家（输入名称）",            ST::Action},
        }},
    };
}

// ── 常用竞技预设 ─────────────────────────────────────────────────────────────
inline std::vector<std::pair<std::string, std::string>> get_presets() {
    return {
        {"竞技模式 (ESL)",
         "mp_restartgame 1; mp_autoteambalance 1; mp_limitteams 2; "
         "mp_maxrounds 30; mp_roundtime 1.75; mp_freezetime 15; "
         "mp_buytime 20; mp_startmoney 800; mp_halftime 1; "
         "mp_match_can_clinch 1; mp_overtime_enable 1; "
         "mp_overtime_maxrounds 6; mp_overtime_startmoney 10000; "
         "mp_player_cashawards 1; mp_team_cashawards 1"},

        {"休闲模式",
         "mp_restartgame 1; mp_autoteambalance 1; mp_limitteams 2; "
         "mp_maxrounds 0; mp_roundtime 5; mp_freezetime 0; "
         "mp_buytime 45; mp_startmoney 1000; mp_halftime 0; "
         "mp_match_can_clinch 0"},

        {"死斗模式",
         "mp_restartgame 1; mp_maxrounds 1; mp_freezetime 0; "
         "mp_buytime 0; mp_startmoney 16000; mp_halftime 0; "
         "mp_match_can_clinch 0; mp_respawn_on_death_ct 1; "
         "mp_respawn_on_death_t 1; mp_respawnwavetime_ct 0; "
         "mp_respawnwavetime_t 0"},

        {"双人竞技",
         "mp_restartgame 1; mp_maxrounds 16; mp_roundtime 1.75; "
         "mp_freezetime 15; mp_buytime 20; mp_startmoney 800; "
         "mp_halftime 1; mp_overtime_enable 1; "
         "mp_overtime_maxrounds 6; mp_autoteambalance 1"},

        {"回防练习",
         "mp_restartgame 1; mp_maxrounds 1; mp_roundtime 10; "
         "mp_freezetime 0; mp_buytime 5; mp_startmoney 16000; "
         "mp_halftime 0; mp_match_can_clinch 0"},

        {"练习 / 热身",
         "mp_restartgame 1; mp_maxrounds 0; mp_roundtime 60; "
         "mp_freezetime 0; mp_buytime 0; mp_startmoney 16000; "
         "mp_halftime 0; mp_match_can_clinch 0; "
         "mp_warmuptime 60; mp_warmup_start; "
         "sv_cheats 1; sv_grenade_trajectory 1; sv_showimpacts 1; "
         "bot_add; bot_difficulty 2; bot_quota 4"},
    };
}
