# 地块标记与世界色彩

## 游玩中的外观

- `AGroundBlock` 和 `AWorldsBasicActor` 的可踩面中心带灰白圆环十字，使用无碰撞的 `CenterMarker` 组件。标记随地块升降、旋转和重力方向对齐，常规直径 36 cm，小地块自动缩小。
- 初始 SOIL06、激活后的 SOIL10、草皮和草叶仍按原机制切换；游玩中地块不再使用 Stencil=1 保色，因此激活前后都经过灰白后处理。
- 草叶中心预留半径 30 cm 的标记区域，其余区域保持原密度。
- 压力板仅板面保持彩色：升降荧光红、弹射荧光绿、重力荧光紫、终点荧光黄。底座保持灰白。
- `M_PressurePlateGlow` 保留曝光补偿；`ApplyPlateColor()` 为未按下/按下状态分别写入 `GlowStrength=3.0/1.8`，按下颜色亮度为原色的 65%。

## 主界面展示地块

`L_Tutorial` 公园展示地块 `BP_GroundBlock` 单独带 `OpeningMenuColor` Actor Tag。`LoopSessionSubsystem` 在显示主菜单时启用其主体、草皮与草叶的 Stencil=1 保色，开始游戏或离开菜单时关闭。其他地块不受影响，Settings 页面仍属于主菜单；暂停菜单不会打开此开关。长椅与路灯原有的保色设置保持原样。

## 通关恢复色彩

`ULoopWorldColorSubsystem` 使用 `MPC_WorldColor` 驱动现有 `M_PP_MonochromeSelective`：

| 参数 | 默认 | 最终通关 |
| --- | --- | --- |
| WorldColorRestored | 0：灰白环境与局部保色 | 1：整个场景恢复原色 |
| RestorationSaturation | 1.0 | 瞬间升至 1.65，2.5 秒平滑回落至 1.0 |

最终目标板启用 `Restore World Color On Reach`，按下后先恢复色彩，再广播原有 `OnGoalReached`。同一世界重复触发不重播饱和度变化。其他蓝图也可通过 World Subsystem 的 `RestoreWorldColor` 调用相同效果。

- 当前 Main 的正式终点启用该选项。
- Main/Tutorial 的样板区目标板、教学关卡与 MovementTest 中的目标板关闭此选项，避免教学或试踩提前结束世界色彩流程。
- 从目标板蓝图新放置的实例默认启用；中途目标需手动关闭，只在最终终点保留。
- 世界恢复状态写入 `LoopSaveGame`。Continue 恢复通关后的自然彩色，不重播饱和度变化；旧存档缺省为灰白，旧版存档中的地块 Stencil=1 也不会重新使踩过的地块变彩色。Restart/NewGame 重置当前世界色彩。

## 资源与检查

新增项目材质 `M_BlockCenterMarker` 与 `PostProcess/MPC_WorldColor`，未修改第三方土壤或草叶源材质。已同步 Main、Tutorial、MovementTest 和对应蓝图默认值，保留所有现有地块的位置、旋转与缩放。

工作区 `WorldColorAudit` 保存修改前的地图、蓝图和材质备份，以及材质配置、地图样式和 PIE 验证报告。PIE 使用独立存档槽 `CodexWorldColorVerification`，结束后删除，不覆盖玩家存档。
