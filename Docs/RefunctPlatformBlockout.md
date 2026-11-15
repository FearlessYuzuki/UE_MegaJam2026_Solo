# Main Refunct 风格基础地块

本轮调整 `/Game/MegaJam2026/Maps/Gameplay/L_Main` 中已生成的半环灰盒，保留上下世界、起点、重力切换点以及压力板逻辑。空间结构仍为下层半环与反向的上层半环，不是连续扭转曲面。

## 地块与间距

- 共 107 个地块实体：82 个岛块、4 个端点平台、16 个远景地标柱、4 个升降支线块、1 个弹射高台落点。
- 保留半径 55 m 的中心动线，删去 90 个冗余岛块，降低密度。
- 每层端点附近的小岛由端点平台替代，避免大平台包住小地块。
- 侧向岛块增加 3.3–6.3 m 高差；主线沿用 1.8–3.6 m 的高度节奏。
- 升降支线方块沿支线旋转到 135°，避免斜向排列时角部相交。
- 弹射板下一段外侧补充离水 8 m 的高台，约 5.8 m 水平距离；主线可绕过，支线可用弹射抵达后跳回主线。
- 出生点中心为 `(-400, -5500, -19655)` cm，按现有角色 160 cm 胶囊半高留出落地空间，避免生成时嵌入端点平台。
- 以每块模型的实际世界变换进行有向矩形分离轴检查，同时检查 Z 区间。检查对象为地块实体，不包括允许接触平台的压力板、水面和触发体。

## 顶面材质转换

材质目录为 `/Game/MegaJam2026/Materials/Refunct`。

- `M_RefunctPlatform`：根据模型的局部上方向区分顶面与侧壁，上层倒置时仍以朝下的可踩面为顶面。
- `MI_Refunct_Unvisited`：暖白顶面与冷灰石材侧壁。
- `MI_Refunct_Visited`：橙色顶面，侧壁保持原色。
- `MI_Refunct_Landmark`：深灰远景柱。

87 个常规岛块、端点平台和弹射高台使用已有 `MaterialTriggerBlock` 类，仅玩家控制的 Pawn 可触发。触发盒位于可踩面上方 5–85 cm，水平范围比平台内缩 12 cm。踩过后保留橙色至本次关卡运行结束，离开不会恢复，也不会改变角色重力。重新运行关卡恢复未踩状态，尚未加入跨存档的地块染色进度。

这 87 个地块的网格开启 CustomDepth、Stencil=1，复用关卡已有 `M_PP_MonochromeSelective` 的可见像素保色；不修改全场黑白后处理。材质只改变局部可踩面，侧壁保留石材色，不增加每帧材质更新。

4 个升降块继续使用已有 GroundBlock，保留原先的升降与碰撞启用逻辑，本轮只调整朝向和石材外观。

## 编辑与备份

生成物新增 `Refunct_Blockout_v1` 标签，触发型地块另有 `Refunct_TouchSurface` 标签，保留原来的 Actor 标签和大纲文件夹。

调整前分别备份磁盘上的 `L_Main.umap` 和编辑器内存中的关卡 T3D，备份位置记在 `refunct_layout_result.json`。原有 `BuildHalfMobiusBlockout.py` 仍是旧版灰盒生成器；本轮变更由 `apply_refunct_layout.py` 与 `refunct_layout_plan.json` 应用，禁止在已有生成标签的地图上重复执行。

结构验证覆盖实体尺寸、碰撞、上下朝向、触发盒范围、材质绑定、唯一出生点与全部实体的重叠检查。完整动线的跳跃手感与所有关卡机制仍需另行试玩。

本轮 PIE 验证通过：从默认出生点正常站上端点平台；实际踩升降板让 4 块支线各升起 600 cm 并恢复碰撞；以约 278 cm/s 的进板水平速度，经真实弹射触发以 1200 cm/s 上射并落到 8 m 高台；实际踩重力板后自然穿越到上层端点平台并进入 Walking。地块踩后保色、离开保留、重复进入不翻重力、上层倒置触发均已检查。测试会瞬移到机制入口，不等同于从起点手动跑通整条路线。

报告：`Saved/refunct_layout_verification.json`、`Saved/refunct_pie_verification.json`、`Saved/refunct_gameplay_verification.json`。编辑脚本与修改前备份保留在 `D:/Work/GameDev/BlenderProject/GameJam2026Epic`。
