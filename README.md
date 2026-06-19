> 💡 **声明：** 本项目灵感来源于 [fpaezf/CS2-RCON-Tool-V2](https://github.com/fpaezf/Counter-Strike-dedicated-server-admin-tool)，属于纯 **vibe coding** 作品喵～ 如果觉得不好用的话，可以去支持一下原项目哦！

# CS2 RCON Server Manager 🐱

一个 **Source RCON 协议** 的 CS2 服务器管理面板，使用 **C++17** + **Dear ImGui** + **GLFW** + **OpenGL** 开发的说～喵！

## ✨ 功能一览

- 🔌 **RCON 连接管理** — 输入 IP:Port + 密码就能连上服务器哦～
- 📊 **仪表盘** — 状态卡片、预设切换、快捷操作，一目了然喵！
- ⚙️ **全面设置** — CS2 参数随便调喵～
  - 回合/比赛设置 (mp_roundtime, mp_maxrounds, mp_freezetime 等等)
  - 经济系统 (mp_startmoney, mp_maxmoney 什么的)
  - 热身/加时赛控制
  - 玩法规则 (友军伤害、自动平衡、购买限制……)
  - 武器/投掷物设置
  - Bot 管理 (添加/踢出/难度)
  - 服务器/地图管理
- 👤 **玩家管理** — 看看谁在服务器里，还能踢出/封禁喵！
- 🗺️ **地图切换** — 常用地图一键切换，工坊地图也支持的说～
- 📟 **控制台** — 完整命令输入/输出终端，想输啥输啥喵
- ⚡ **预设配置** — 竞技(ESL)、休闲、死斗、双人竞技、练枪……一键切换！
- 🎨 **精美暗色主题** — 现代扁平化 UI，好看又护眼喵～

## 系统要求

- Windows 10/11
- 支持 OpenGL 3.0+ 的显卡
- CS2 服务器（记得开启 RCON 哦～）

## 快速开始

### 下载

直接双击 `cs2_rcon_panel.exe` 就能用啦～（只有约 670KB，不需要安装什么东西喵）

### 从源码构建

**需要准备：**
- Visual Studio 2022（要装 C++ 桌面开发组件哦）
- CMake 3.16+

**构建步骤：**

```bash
# 方法一：双击 build.bat 就好啦～
build.bat

# 方法二：手动构建喵
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -B build -G "Visual Studio 17 2022" -A x64 .
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release
```

### 使用指南

1. 启动程序
2. 输入 CS2 服务器地址（格式：`IP:端口`，默认端口 27015 喵）
3. 输入 RCON 密码（在服务器 `server.cfg` 里用 `rcon_password` 设置的哦）
4. 点击 **Connect to Server**
5. 连上之后就能愉快地管理服务器啦～！

> 💡 **小提示：** CS2 服务器要启用 RCON 才行，在 `server.cfg` 里加上：
> ```
> rcon_password 你的密码
> ```

## 预设配置

| 预设 | 说明 |
|------|------|
| **Competitive (ESL)** | 标准竞技模式，30局，1.75分钟回合 |
| **Casual** | 休闲模式，5分钟回合，不用冻结喵 |
| **Deathmatch** | 死斗模式，复活不用等～ |
| **Wingman** | 双人竞技，16局制 |
| **Retakes** | 回防练习，练枪好帮手！ |
| **Practice / Warmup** | 练枪模式，开作弊+无限时间，随便玩喵～ |

## 项目结构

```
cs2_rcon_panel/
├── src/
│   ├── main.cpp              # 入口、窗口、主循环
│   ├── rcon_client.h/cpp     # Source RCON 协议实现
│   ├── settings.h            # CS2 服务器命令定义
│   ├── theme.h/cpp           # 自定义 UI 主题
│   └── ui.h/cpp              # 完整 UI 渲染
├── deps/imgui/               # Dear ImGui 源码
├── build.bat                 # 一键构建脚本
└── CMakeLists.txt            # CMake 配置
```

## RCON 协议

用的 Valve 的 Source RCON 协议喵～
- 基于 TCP
- 小端序数据包
- 认证 → 执行命令 → 获取响应
- 不需要额外库，Windows Sockets 原生搞定！

## 许可证

MIT 喵～ 随便用随便改的说！
