# DungeonGenerator 程序化地牢生成插件

**版本：1.0.0 | 开发者：PinguWaddle | 引擎版本：Unreal Engine 5.5+**

> ⚠️ 本插件目前处于**第一阶段开发完成**状态，核心功能已可正常使用，但仍有许多功能和优化尚在规划中，后续版本会持续更新和完善。欢迎反馈问题和建议。

---

## 简介

DungeonGenerator 是一款面向 Unreal Engine 5 的程序化地牢生成插件。通过随机撒点、物理分离、最小生成树等算法，自动生成具有自然布局的地牢关卡，并内置房间类型系统和 Blueprint 查询接口，方便开发者在此基础上进行玩法扩展。

---

## 功能特性

- **程序化地牢生成** — 基于圆形随机撒点 + 物理分离 + Delaunay 三角剖分 + 最小生成树算法
- **房间类型系统** — 支持起始房、Boss房、普通宝箱房、稀有宝箱房、史诗宝箱房、NPC交互房、普通房间
- **可视化调试** — 编辑器内不同房间类型显示不同颜色的调试框
- **Blueprint 友好** — 完整的查询接口和事件委托，无需修改 C++ 代码即可扩展玩法
- **Actor 生成支持** — 可为不同房间类型配置对应的 Spawn Actor 类
- **ISM 渲染** — 使用 Instanced Static Mesh 渲染，性能友好
- **编辑器工具** — 支持一键生成、清除、随机种子

---

## 安装方法

1. 将 `DungeonGenerator` 文件夹复制到你的项目的 `Plugins/` 目录下
2. 右键点击 `.uproject` 文件，选择 **Generate Visual Studio Project Files**
3. 重新编译项目
4. 打开 UE5 编辑器，在 **Edit → Plugins** 中确认 `Dungeon Generator` 插件已启用

---

## 快速开始

### 1. 放置 Actor

在 Content Browser 中搜索 **DungeonGeneratorActor**，将其拖入场景。

### 2. 配置参数

在 Details 面板中配置以下参数：

| 分类 | 参数 | 说明 |
|------|------|------|
| Layout | Random Seed | 随机种子，固定种子 = 固定地牢 |
| Layout | Num Rooms | 初始生成的房间总数 |
| Layout | Spawn Radius | 房间初始散布的圆形半径 |
| Rooms | Room Min/Max Size | 房间最小/最大尺寸（单位：cm）|
| Rooms | Main Room Size Multiplier | 主房间筛选阈值，越大保留的房间越少越大 |
| Corridors | Corridor Width | 走廊宽度（单位：cm）|
| Corridors | Extra Loop Chance | 额外环路概率（0~1），增加地牢回路 |
| RoomTypes | Treasure Config | 各等级宝箱房数量 |
| RoomTypes | NPC Config | NPC 交互房间数量 |
| Meshes | Floor/Wall Mesh | 地板和墙壁的 Static Mesh |
| Spawning | 各类 Spawn Class | 不同房间类型对应生成的 Actor 类 |
| Debug | Show Room Type Debug | 是否显示房间类型调试框（仅编辑器）|

### 3. 生成地牢

点击 Details 面板中的 **GenerateDungeon** 按钮即可生成地牢。

---

## Blueprint 使用

### 查询接口

在 Blueprint 中获取 `DungeonGeneratorActor` 引用后，可调用以下函数：

- `GetStartRoom()` — 获取起始房间数据
- `GetBossRoom()` — 获取 Boss 房间数据
- `GetRoomsByType(Type)` — 获取指定类型的所有房间
- `GetAllRooms()` — 获取所有房间数据
- `GetRoomByID(ID)` — 通过 ID 获取指定房间

### 事件委托

- `OnDungeonGenerated` — 地牢生成完成时触发，携带所有房间数据
- `OnDungeonCleared` — 地牢清除时触发

### 房间数据（FDungeonRoom）

每个房间包含以下数据：

| 字段 | 类型 | 说明 |
|------|------|------|
| ID | int32 | 房间唯一标识 |
| Position | FVector2D | 房间左下角位置 |
| Size | FVector2D | 房间尺寸 |
| WorldCenter | FVector | 房间中心世界坐标（3D）|
| Area | float | 房间面积 |
| RoomType | ERoomType | 房间类型枚举 |

---

## 房间类型颜色对照

| 颜色 | 房间类型 |
|------|----------|
| 🟢 绿色 | Start 起始房 |
| 🔴 红色 | Boss 房 |
| 🟡 黄色 | Treasure Common 普通宝箱 |
| 🔵 蓝色 | Treasure Rare 稀有宝箱 |
| 🟣 紫色 | Treasure Epic 史诗宝箱 |
| 🩵 青色 | NPC 交互房 |
| ⬜ 白色 | Normal 普通房 |

---

## 已知问题 / 待优化

- 走廊转角处存在轻微锯齿边缘
- 房间数量较多时（>80）生成速度较慢
- 目前不支持多层地牢（高度差）

---

## 后续规划

- [ ] 多层地牢支持（高度差 + 楼梯）
- [ ] 次要房间纳入地牢（走廊经过的小房间）
- [ ] 房间内陈设系统（道具、光源、装饰物自动摆放）
- [ ] 不同房间类型的 Buff/Debuff 系统
- [ ] 更多走廊风格（曲线走廊、宽走廊等）
- [ ] 运行时生成优化（异步生成）
- [ ] 小地图生成支持

---

## 更新日志

### v1.0.0（2026-04-10）
- 完成第一阶段开发
- 实现基础程序化地牢生成算法
- 实现房间类型系统
- 实现 Blueprint 查询接口和事件委托
- 实现 Actor Spawn 支持
- 实现编辑器调试可视化

---

## 联系方式

如有问题或建议，欢迎通过以下方式联系：

- 开发者：PinguWaddle
