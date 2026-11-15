# 游戏内暂停与存档

## 主菜单

`GameDefaultMap` 现在指向 `L_Tutorial`。`BP_MegaJamPlayerController` 会在本地控制器开始时显示 `WBP_MainMenuSplitExample`，因此新游戏入口不再依赖手动放置菜单。

- `NewGame`：清除继续状态并重新打开教学关卡。
- `Continue`：读取 `LoopProgress`，恢复地图、玩家位置、控制旋转、重力方向、速度、蹲伏状态、检查点，以及已保存的 GroundBlock、压力板和区域控制器状态。
- 没有有效存档时 Continue 禁用。

## ESC 菜单

游戏中按 `Escape` 显示 `WBP_PauseMenuUI`，暂停世界并锁定鼠标到 UI：

- `Resume`：恢复游戏。
- `Settings`：打开原有设置页，返回后仍保持暂停。
- `Save&Exit`：写入 `LoopProgress` 后返回教学主菜单。
- `Restart`：重新加载当前关卡，不读取存档。

暂停菜单沿用 Loop/莫比乌斯环的居中构图和 `M_UI_MenuSweep` 扫光。暂停状态下扫光使用真实帧时间，所以鼠标悬停动画不会停住。点击音效使用已存在的 Resources 资产：单击音效用于普通选项，双击音效用于保存退出和重启。

存档使用版本号 `1`。存档写入 `%Project%/Saved/SaveGames/LoopProgress.sav`，测试用的临时存档已清理。Save&Exit 默认回到主菜单；`LoopSessionSubsystem.bQuitOnSaveExit` 可在蓝图或 C++ 中改为退出程序。

新游戏与 Restart 会重置当前游玩状态，但不会立即删除旧的磁盘存档；下一次保存退出时覆盖单槽进度。保存失败会停留在暂停菜单并显示提示。

运行验证使用独立的 `CodexPauseMenuVerification` 槽，已验证新游戏、暂停、设置返回、Resume、翻转重力存读档、地块位置恢复、保存返回主菜单与 Restart，测试槽已删除。鼠标悬停时记录到暂停状态下 SweepPhase 从 0 连续过渡到 1。
