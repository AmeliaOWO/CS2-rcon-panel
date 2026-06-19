#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

// ── A single command entry for auto-complete ────────────────────────────────
struct ConsoleCmd {
    std::string cmd;         // command name (e.g. "mp_restartgame")
    std::string params;      // parameter hint (e.g. "<seconds>")
    std::string desc;        // description
    std::string category;    // category name
};

// ── All known CS2 commands ───────────────────────────────────────────────────
inline const std::vector<ConsoleCmd>& get_all_commands() {
    static const std::vector<ConsoleCmd> cmds = {
        // ── Server Info ──
        {"status",          "",                     "查看服务器状态、玩家列表、Steam ID",            "服务器信息"},
        {"users",           "",                     "查看 RCON 在线用户",                             "服务器信息"},
        {"maps",            "",                     "列出服务器所有地图",                             "服务器信息"},
        {"version",         "",                     "查看 CS2 服务器版本",                            "服务器信息"},
        {"stats",           "",                     "查看服务器统计信息",                             "服务器信息"},

        // ── Match / Round ──
        {"mp_restartgame",  "<秒数>",               "在 N 秒后重启比赛",                             "比赛控制"},
        {"mp_warmup_end",   "",                     "结束热身阶段",                                  "比赛控制"},
        {"mp_warmup_start", "",                     "开始热身",                                      "比赛控制"},
        {"mp_warmup_pausetimer", "1",               "暂停热身计时器",                                "比赛控制"},
        {"mp_pause_match",  "",                     "暂停比赛",                                      "比赛控制"},
        {"mp_unpause_match","",                     "恢复比赛",                                      "比赛控制"},
        {"mp_pause",        "",                     "暂停比赛计时器",                                "比赛控制"},
        {"mp_unpause",      "",                     "恢复比赛计时器",                                "比赛控制"},
        {"mp_halftime",     "<0/1>",                "启用/禁用半场交换",                             "比赛控制"},
        {"mp_match_can_clinch","<0/1>",             "允许锁定胜局",                                  "比赛控制"},
        {"mp_overtime_enable","<0/1>",              "启用加时赛",                                    "比赛控制"},

        // ── Round CVars ──
        {"mp_maxrounds",    "<数量>",               "设置最大回合数（默认 30）",                     "回合设置"},
        {"mp_roundtime",    "<分钟>",               "回合时长（默认 1.92）",                         "回合设置"},
        {"mp_roundtime_defuse","<分钟>",             "拆弹地图回合时长",                              "回合设置"},
        {"mp_roundtime_hostage","<分钟>",            "人质地图回合时长",                              "回合设置"},
        {"mp_timelimit",    "<分钟>",               "地图时间限制（0=不限）",                        "回合设置"},
        {"mp_freezetime",   "<秒>",                 "冻结时间（默认 15）",                           "回合设置"},
        {"mp_buytime",      "<秒>",                 "购买时间（默认 20）",                           "回合设置"},
        {"mp_round_restart_delay","<秒>",           "回合重启延迟",                                  "回合设置"},
        {"mp_c4timer",      "<秒>",                 "C4 爆炸倒计时（默认 40）",                      "回合设置"},

        // ── Economy ──
        {"mp_startmoney",   "<金额>",               "起始金钱（默认 800）",                          "经济设置"},
        {"mp_maxmoney",     "<金额>",               "最大金钱（默认 16000）",                         "经济设置"},
        {"mp_playercashawards","<0/1>",             "玩家行为金钱奖励",                              "经济设置"},
        {"mp_teamcashawards","<0/1>",               "团队行为金钱奖励",                              "经济设置"},

        // ── Gameplay ──
        {"mp_friendlyfire", "<0/1>",                "友军伤害开关",                                  "游戏规则"},
        {"mp_autoteambalance","<0/1>",              "自动平衡队伍",                                  "游戏规则"},
        {"mp_limitteams",   "<数量>",               "队伍最大人数差",                                "游戏规则"},
        {"mp_buy_anywhere", "<0/1>",                "随地购买武器",                                  "游戏规则"},
        {"mp_buy_during_immunity","<0/1>",          "免疫期间购买",                                  "游戏规则"},
        {"sv_gravity",      "<数值>",               "重力（默认 800）",                              "游戏规则"},
        {"sv_cheats",       "<0/1>",                "启用作弊命令",                                  "游戏规则"},
        {"sv_infinite_ammo","<0-2>",                "无限弹药（1=弹夹 2=备弹）",                    "游戏规则"},
        {"sv_password",     "<密码>",               "设置服务器密码",                                "游戏规则"},

        // ── Weapons / Grenades ──
        {"ammo_grenade_limit_total","<数量>",       "最大投掷物数（默认 4）",                        "武器设置"},
        {"ammo_grenade_limit_flashbang","<数量>",   "最大闪光弹数（默认 2）",                        "武器设置"},
        {"ammo_grenade_limit_smokes","<数量>",      "最大烟雾弹数（默认 1）",                        "武器设置"},
        {"ammo_grenade_limit_incendiary","<数量>",  "最大燃烧弹数（默认 1）",                        "武器设置"},
        {"ammo_grenade_limit_decoy","<数量>",       "最大诱饵弹数（默认 1）",                        "武器设置"},
        {"sv_grenade_trajectory","<0/1>",           "显示投掷物轨迹（需作弊）",                      "武器设置"},
        {"sv_showimpacts",  "<0/1>",                "显示弹道命中点（需作弊）",                      "武器设置"},
        {"sv_rethrow_last_grenade","",              "重扔上一颗投掷物（需作弊）",                    "武器设置"},
        {"sv_grenade_trajectory_prac_pipreview","<0/1>","投掷物轨迹预览",                             "武器设置"},

        // ── Player Management ──
        {"kick",            "<名称/ID>",            "踢出玩家",                                      "玩家管理"},
        {"kickid",          "<UserID>",             "按 UserID 踢出玩家（status 查看）",             "玩家管理"},
        {"ban",             "<名称>",               "封禁玩家",                                      "玩家管理"},
        {"banid",           "<SteamID>",            "按 SteamID 封禁玩家",                           "玩家管理"},
        {"removeid",        "<SteamID>",            "解除 SteamID 封禁",                             "玩家管理"},
        {"say",             "<消息>",               "向所有玩家发送聊天消息",                         "玩家管理"},

        // ── Bots ──
        {"bot_add",         "",                     "添加机器人",                                    "机器人"},
        {"bot_add_t",       "",                     "添加 Terrorist 机器人",                         "机器人"},
        {"bot_add_ct",      "",                     "添加 Counter-Terrorist 机器人",                  "机器人"},
        {"bot_kick",        "",                     "踢出所有机器人",                                "机器人"},
        {"bot_kill",        "",                     "杀死所有机器人",                                "机器人"},
        {"bot_place",       "",                     "在准星位置放置机器人",                           "机器人"},
        {"bot_stop",        "<0/1>",                "停止/恢复机器人行动",                           "机器人"},
        {"bot_difficulty",  "<0-3>",                "机器人难度（0=简单 3=专家）",                   "机器人"},
        {"bot_quota",       "<数量>",               "设置机器人数量",                                "机器人"},
        {"bot_quota_mode",  "<fill/normal>",        "机器人配额模式",                                "机器人"},
        {"bot_join_after_player","<0/1>",           "玩家加入后添加机器人",                          "机器人"},

        // ── Maps ──
        {"changelevel",     "<地图名>",             "切换地图（不重启服务器）",                      "地图管理"},
        {"map",             "<地图名>",             "切换地图（完全重启）",                          "地图管理"},
        {"ds_workshop_changelevel","<ID>",          "加载工坊地图",                                  "地图管理"},

        // ── Server ──
        {"hostname",        "<名称>",               "设置服务器名称",                                "服务器"},
        {"exec",            "<文件名>",             "执行服务器配置文件 (.cfg)",                      "服务器"},
        {"sv_maxplayers",   "<数量>",               "设置最大玩家数",                                "服务器"},
        {"sv_visiblemaxplayers","<数量>",           "浏览器显示的最大玩家数",                         "服务器"},

        // ── Cheats / Practice ──
        {"noclip",          "",                     "穿墙模式（需作弊）",                            "练枪作弊"},
        {"god",             "",                     "无敌模式（需作弊）",                            "练枪作弊"},
        {"sv_showimpacts",  "<0/1/2>",              "显示弹道命中点（2=穿透显示）",                  "练枪作弊"},
        {"host_timescale",  "<倍率>",               "游戏速度倍率（1.0=正常）",                     "练枪作弊"},
        {"mp_warmup_pausetimer","<0/1>",            "暂停热身计时器",                               "练枪作弊"},
        {"bot_place",       "",                     "放置机器人（需作弊）",                          "练枪作弊"},

        // ── ESL / Competitive Presets ──
        {"exec esl5on5.cfg","",                     "加载 ESL 5v5 竞技配置",                        "预设"},
        {"exec gamemode_competitive","",            "加载竞技模式配置",                              "预设"},
        {"exec gamemode_casual","",                 "加载休闲模式配置",                              "预设"},
        {"exec gamemode_deathmatch","",             "加载死斗模式配置",                              "预设"},
    };
    return cmds;
}

// ── Command auto-complete logic ─────────────────────────────────────────────
inline std::vector<const ConsoleCmd*> match_commands(const std::string& input) {
    if (input.empty()) return {};

    std::string lower_input = input;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });

    std::vector<const ConsoleCmd*> results;
    const auto& all = get_all_commands();

    for (const auto& cmd : all) {
        // Match if command starts with what user typed (case-insensitive)
        std::string lower_cmd = cmd.cmd;
        std::transform(lower_cmd.begin(), lower_cmd.end(), lower_cmd.begin(),
            [](unsigned char c) { return (char)std::tolower(c); });
        if (lower_cmd.find(lower_input) == 0) {
            results.push_back(&cmd);
        }
    }

    return results;
}
