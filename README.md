# CS2 RCON Server Manager

一个基于 **Source RCON 协议** 的 CS2 服务器管理面板，使用 **C++17** + **Dear ImGui** + **GLFW** + **OpenGL** 开发。

## 功能

- 🔌 **RCON 连接管理** — 通过 IP:Port + 密码连接 CS2 服务器
- 📊 **仪表盘** — 快速状态卡片、预设模式一键切换、快捷操作
- ⚙️ **全面设置** — 按类别组织的 CS2 服务器参数调节面板
  - 回合/比赛设置 (mp_roundtime, mp_maxrounds, mp_freezetime, etc.)
  - 经济系统 (mp_startmoney, mp_maxmoney, etc.)
  - 热身/加时赛控制
  - 玩法规则 (友军伤害, 自动平衡, 购买限制等)
  - 武器/投掷物设置
  - Bot 管理 (添加/踢出/难度)
  - 服务器/地图管理
- 👤 **玩家管理** — 玩家列表、踢出/封禁
- 🗺️ **地图切换** — 常用地图一键切换 + 工坊地图支持
- 📟 **控制台** — 完整的命令输入/输出终端
- ⚡ **预设配置** — 竞技(ESL)、休闲、死斗、双人竞技、练枪等模式快速切换
- 🎨 **精美暗色主题** — 现代扁平化 UI 设计

## 系统要求

- Windows 10/11
- 支持 OpenGL 3.0+ 的显卡
- CS2 服务器（已启用 RCON）

## 快速开始

### 下载

直接运行 `cs2_rcon_panel.exe`（约 670KB，无需额外安装）。

### 构建（从源码）

**前置条件：**
- Visual Studio 2022（含 C++ 桌面开发组件）
- CMake 3.16+

**构建步骤：**

```bash
# 方法一：双击 build.bat
build.bat

# 方法二：手动构建
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -B build -G "Visual Studio 17 2022" -A x64 .
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release
```

### 使用

1. 启动程序
2. 输入 CS2 服务器地址（格式：`IP:端口`，默认端口 27015）
3. 输入 RCON 密码（在服务器 `server.cfg` 中通过 `rcon_password` 设置）
4. 点击 **Connect to Server**
5. 连接成功后即可管理服务器

> 💡 **提示：** CS2 服务器需要启用 RCON，在 `server.cfg` 中添加：
> ```
> rcon_password 你的密码
> ```

## 预设配置说明

| 预设 | 用途 |
|------|------|
| **Competitive (ESL)** | 标准竞技模式，30局，1.75分钟回合 |
| **Casual** | 休闲模式，5分钟回合，无需冻结 |
| **Deathmatch** | 死斗模式，重生无等待 |
| **Wingman** | 双人竞技，16局制 |
| **Retakes** | 回防练习模式 |
| **Practice / Warmup** | 练枪模式，开启作弊+无限时间 |

## 架构

```
cs2_rcon_panel/
├── src/
│   ├── main.cpp              # 入口、窗口、主循环
│   ├── rcon_client.h/cpp     # Source RCON 协议实现
│   ├── settings.h            # CS2 服务器命令定义
│   ├── theme.h/cpp           # 自定义 UI 主题
│   └── ui.h/cpp              # 完整 UI 渲染
├── deps/imgui/               # Dear ImGui 源码
├── build.bat                 # 构建脚本 (VS2022)
└── CMakeLists.txt            # CMake 构建配置
```

## RCON 协议

使用 Valve 的 Source RCON 协议：
- 基于 TCP
- 小端序数据包
- 认证 → 执行命令 → 获取响应
- 无需额外库，使用 Windows Sockets 原生实现

## 许可证

MIT
