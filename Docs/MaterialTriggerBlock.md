# Material Trigger Block

`AMaterialTriggerBlock` 继承 `AWorldsBasicActor`，复用底座、TriggerBox 和材质处理，只关闭进入时的重力翻转与视角翻转。原有 `BP_WorldsBasicActor` 保持原行为。

## 蓝图资源

- 路径：`/Game/MegaJam2026/Blueprints/Actors/BP_MaterialTriggerBlock`。
- 直接父类：`AMaterialTriggerBlock`。
- 默认材质、底座尺寸和 TriggerBox 的相对变换沿用已有 `BP_WorldsBasicActor`。
- 本次只生成资源，不自动替换或摆放关卡中的方块。

## 行为与配置

- 只接受 `IsPlayerControlled()` 的 Pawn；普通 Actor 和未被玩家控制的 Pawn 不触发。
- 进入后给自身 `BaseBox` 的 `MaterialSlot` 切换 `OverlapMaterial`；重叠期间重复进入不会反复执行。
- `IdleMaterial` 控制普通材质，`bRestoreOnEndOverlap` 默认开启；离开后恢复进入前的材质。
- 保留 `bUseOverlapTint`、颜色参数、蓝图进入／离开事件和本地事件委托。
- 不修改角色的重力方向、相机旋转或移动模式，无 Actor Tick。

## 验证范围

UE 5.8 Win64 Development Editor 编译及热重载通过；蓝图编译通过，父类和保存后的组件配置已核对。本次未执行 PIE 进入／离开实测。
