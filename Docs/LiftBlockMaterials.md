# 升降地块与材质选择

本轮关卡修改仅保存到 `/Game/MegaJam2026/Maps/Dev/MovementTest`，没有调整 Main 的布局。

## 使用方式

普通踩踏地块使用 `/Game/MegaJam2026/Blueprints/Actors/BP_BasicGroundBlock`。它继承 `BP_GroundBlock`，关闭 `Requires Lift`，不需要压力板，进入游戏立即可踩；踩过后主体为 SOIL10、顶面为草皮并显示草03，未踩时为 SOIL06。游玩中仍保持灰白，最终通关后显示原色；倒置放置时也可从对应重力方向踩踏。普通地块不会响应升降区域控制器。

原 `BP_GroundBlock` 保留 `Requires Lift=true` 的升降用途。`BP_MaterialTriggerBlock` 是旧版整块材质替换地块，没有独立草皮面与草叶组件。

`AGroundBlock` / `BP_GroundBlock` 新增以下实例参数：

| 参数 | 用途 |
| --- | --- |
| Initial Material | 未踩过时的材质；本轮按选择使用 SOIL06，内部属性名仍为 `WhiteMaterial` 以兼容已有资产 |
| Land Material | 踩过后的主体材质，侧壁显示土地 |
| Grass Material | 踩过后局部上表面的草皮材质 |
| Grass Mesh | 顶面草叶网格，默认已选的 `lowGrass_01_01_SM`（GRASS MESH 03） |
| Grass Spacing / Grass Scale / Grass Seed | 草丛间距（cm）、单丛缩放和固定分布种子 |
| Use Player Gravity For Lift | 开启后在开始升起时读取玩家 0 的重力，朝其反方向升起并对齐可踩面；中途不再改变方向 |
| Lift Region Tag | 与原来的区域控制器或升地块压力板使用同一个分组名 |

未开启 `Use Player Gravity For Lift` 时保留原有的局部 +Z 方向，适用于设计者手动将上层地块 Roll 设为 180° 的固定布局。本轮新增上下测试组均开启此选项。它按整个地块的局部方向配置草皮面及触发盒，翻转世界中草皮朝下、触发盒也在朝下的可踩面一侧。

升起前与移动中关闭地块碰撞和踩踏触发；到位恢复地块碰撞并开启触发盒。只有玩家控制且重力朝向与可踩面一致的角色能够触发。触发后主体换为 `Land Material`，附着于顶部的无碰撞平面换为 `Grass Material`，离开后保持，重开关卡重置。材质未配置齐时不消费触发，避免出现半套外观。

`IsActivated` 与 `OnBlockActivated` 可用于蓝图监听。Tick 仅在升起过程中启用，材质激活后关闭触发盒；没有持续轮询、每帧创建动态材质或修改角色重力。

激活后主体、顶部与草叶继续经过灰白后处理，直到最终通关恢复整个世界色彩。可踩面增加灰白圆环十字中心标记，详见 `WorldColorAndMarkers.md`。主界面的展示地块仅在菜单显示期间单独保色。

草叶使用独立的 InstancedStaticMesh 组件，随地块旋转但不继承地块的非均匀缩放，草根贴住可踩面。初始和升起阶段均隐藏，踩过后与土地/草皮材质一起显示；上层地块的草朝下生长。每块最多 1024 丛，采用覆盖完整顶面的规则分格与固定种子小幅扰动，草根覆盖到边缘附近，草叶可自然伸过边缘。草叶无碰撞，不影响角色移动，不增加 Actor Tick。草叶保留其源网格自带材质，并启用 Stencil=1 保色。超出近距离范围进行实例距离剔除。

## MovementTest 展示区

大纲目录 `Tests/LiftMaterials`，Actor 标签 `LiftMaterialTest_v1`。展示区从原压力板甲板东侧 X=7900 cm 接壤，中心约 X=11100 cm。

- 下层测试组 `LM_Lower_Block_1/2`，升降板 `LM_Lower_LiftPlate`。
- 上层测试组 `LM_Upper_Block_1/2`，升降板 `LM_Upper_LiftPlate`。
- `LM_Lower_GravityPlate` / `LM_Upper_GravityPlate` 是红方形翻转板，用于两个测试甲板之间往返；红圆形板升起对应分组地块。
- 上下层甲板表面 Z 分别为 0 / 20000 cm。地块局部升起距离为 600 cm。
- 当前用户选定：初始 SOIL06 → 激活后的 SOIL10 侧壁 ＋原草皮地表 ＋ GRASS MESH 03 草叶；草丛间距 24 cm，缩放 1。
- 原有 MovementTest 中的六个升降块也应用相同选型，保留原位置、升降参数及分组。

## 材质列表

所有候选复用项目已导入的资源，未修改第三方源材质。

土地候选目录 `/Game/Ground_Textures_v1/Materials`：

| 编号（样块标牌） | 材质 |
| --- | --- |
| SOIL 01 | `M_Ground_v1_01` |
| SOIL 02 | `M_Ground_v1_02` |
| SOIL 03 | `M_Ground_v1_03` |
| SOIL 04 | `M_Ground_v1_04` |
| SOIL 05 | `M_Ground_v1_05` |
| SOIL 06 | `M_Ground_v1_06` |
| SOIL 07 | `M_Ground_v1_07` |
| SOIL 08 | `M_Ground_v1_08` |
| SOIL 09 | `M_Ground_v1_09` |
| SOIL 10 | `M_Ground_v1_10` |
| SOIL 11 | `M_Ground_v1_11` |

草皮/地面候选目录 `/Game/UltraStylizedGrass/Materials/Ground`：

| 编号（样块标牌） | 材质 |
| --- | --- |
| GRASS 01 | `MI_Ground_Plane_NoVariation` |
| GRASS 02 | `MI_Ground_Plane_NoVariationShadows` |
| GRASS 03 | `MI_Ground_Plane_Orange` |
| GRASS 04 | `MI_Ground_Plane_Variation` |

这些是适合平面地表的草皮/地面实例；植物卡片用的草叶材质未直接铺到方块表面。选择后可直接给地块的 `Land Material` 和 `Grass Material` 赋值。

另在 `Tests/LiftMaterials/GrassMeshes` 增加三种带立体草叶的展示块，在编辑器与游戏中常驻显示，供选择草的造型：

| 草叶样块编号 | 项目已有网格 |
| --- | --- |
| GRASS MESH 01 | `/Game/UltraStylizedGrass/Meshes/GeometryBased/sm_grass_01` |
| GRASS MESH 02 | `/Game/UltraStylizedGrass/Meshes/GeometryBased/sm_grass_05` |
| GRASS MESH 03 | `/Game/PN_FoliageCollection/Meshes/grassMesh/lowGrass_01_01_SM` |

地表材质编号与草叶网格编号独立。当前已选定草叶 03 `lowGrass_01_01_SM`，铺满顶面；该源网格自带小型植被细节。03 号展示块也更新为 SOIL10 侧壁与密铺草叶。

## 验证与备份

源码：`Source/MegaJam_2026/Actors/GroundBlock.h/.cpp`。编辑前 MovementTest 备份在 `D:/Work/GameDev/BlenderProject/GameJam2026Epic/LiftMaterialBackup/MovementTest.before_lift_materials.umap`。

结构检查覆盖材质引用、触发盒尺寸和附着关系、上下分组及候选样块数量。PIE 检查覆盖真实压力板触发、±600 cm 升起、纯白保持至踩踏、土地/草皮材质分配、正常与倒置着地、保色和邻块隔离。测试瞬移到机制入口，不评估关卡路线设计或跳跃节奏。

真实重力板的自然穿越与上层着地也已验证；另将一块上层测试地块在运行时恢复为正立朝向，确认自动方向模式会按翻转后的重力对齐草皮面和触发盒。测试结果保存于 `Saved/lift_material_behavior_verification.json` 和 `Saved/lift_material_structure_verification.json`。

草叶补充验证覆盖激活前隐藏、激活后显示、无碰撞、上下两侧每个草实例的法线方向和草根位置、非均匀缩放隔离及实例上限。报告为 `Saved/grass_mesh_structure_verification.json`，修改前新增备份为 `LiftMaterialBackup/MovementTest.before_grass_meshes.umap`。

密铺与选型修改前的地图备份为 `LiftMaterialBackup/MovementTest.before_selected_palette.umap`。本轮检查还覆盖草根沿两个轴延伸到超过顶面 90% 的范围，以及 SOIL06 → SOIL10 的实际踩踏切换。
