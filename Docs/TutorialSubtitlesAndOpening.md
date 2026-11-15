# 教学开场视角与字幕接口

## 教学关卡开场镜头

关卡：`/Game/MegaJam2026/Maps/Gameplay/L_Tutorial`。

固定摄像机：`Camera_OpeningUI`，位于 Outliner 的 `Cameras` 文件夹，带 `OpeningUICamera` 标签，FOV 为 55 度，对准公园长椅和路灯。可以在编辑器中直接移动、旋转或调整 FOV 来修改构图。

现有 `WBP_MainMenuUI` 显示时，如果当前关卡中存在带该标签的摄像机，会自动切到此镜头，并隐藏原视频背景以显示实时场景。移除菜单时恢复显示菜单前的视角。摄像机不自动抢占游玩视角；具体何时显示菜单、何时开始教学，由台本流程决定。

这组三件场景物体（GroundBlock 的泥土、草皮、草丛，以及长椅、路灯）使用现有 `CustomDepth/Stencil = 1` 规则保留颜色。场景其他物体继续使用原后处理。

路灯放大到原来的 1.65 倍，高约 3.06 米。它有独立的发光材质实例 `MI_OpeningParkLamp` 和真实暖色 PointLight，局部照明半径 850 cm。光源关闭阴影，避免封闭的灯罩遮住内部光源。

## 半透明黑底白字字幕

已提供 `LoopSubtitleSubsystem`（GameInstance Subsystem），可在关卡蓝图或其他蓝图中调用。

1. `Get Game Instance Subsystem`，Class 选择 `LoopSubtitleSubsystem`。
2. 调用 `Show Subtitle`，填入 `English` 和 `Chinese`。
3. 可选填 `Duration`，单位秒。大于 0 会自动清除；0 表示一直显示，直到下一句替换或主动清除。
4. 调用 `Clear Subtitle` 清除字幕及黑底。

字幕位于屏幕下方，白字、60% 不透明度黑底，使用支持中英文的 Noto Sans 字体，并自动换行，背景高度随文本增长。字幕不捕获鼠标或键盘。中英文跟随现有游戏内语言设置，不更改主菜单文字。替换字幕会取消上一句的清除计时；切换关卡会清理旧字幕。

台本来自 `Content/Resources/Assets/Resources/Outro.txt`，中英文保存在 `DT_LoopNarration`。NewGame 的首次落地后自动播放 `Tutorial.Intro`：先显示操作提示，再播放原来的三句开场字幕。读取 Settings 的游戏内语言，未选择语言时默认英文。游玩中切换语言会立即更新当前字幕，后续句子继续使用所选语言。

NewGame 先用覆盖菜单及场景的黑色遮罩在 0.55 秒内淡出，完全黑屏后重载教学关卡。黑屏保持 3 秒，正中央用 Noto Sans 白字显示 `第一幕：“教学”`，英文为 `Act I: "Tutorial"`，跟随玩家的游戏内语言设置。随后玩家进入 Falling，画面在 1.5 秒内从黑屏渐显。首次落地后才播放 Tutorial.Intro，第一句为 `WASD 前进 · Ctrl 下蹲 · 空格跳跃 · 长按空格爬墙`，随后播放原有开场台词。

第一幕现在使用 `Tutorial/ActOne` 下的完整九块教学路线，由 `Act1_Director`（`BP_TutorialOpeningSequence`）控制。首次落地后按游戏时间等待 10 秒，入门平台与红色压力板自动升起；踩红板后，后续三个指定地块同时升起 2200 cm，到各自编辑器摆放的位置。绿色弹射板和紫色翻转板随其所在地块同步移动。完整路线、字幕事件与转场说明见 `Docs/ActOneTutorial.md`。

八段教学字幕按事件进入队列，较快触发后续事件不会截断正在阅读的句子。字幕当前句、剩余时间与队列和控制器进度一起参与存档；暂停不计时，Continue 接着播放。翻转后首次在上方世界落地播放最后一句，读完后出现黄色完成板。踩下完成板通过跨地图黑屏进入 Main，中央显示 `第二幕：“∞”` / `Act II: "∞"`，0.55 秒淡出、全黑保持标题 3 秒、1.5 秒渐显，Main 首次落地后播放 `Main.Landing`。

三个实例启用 `Requires Lift` 和 `Start At Lift Target`。后者表示编辑器位置是升起后的终点，游戏开始时自动沿升起反方向下沉 `Lift Height`，作者可以继续直接拖动地块编辑最终布局。

Continue 恢复存档，不重播开场；Restart 保持重新开始当前关卡的行为。后续地块、压力板及反转段落仍通过 `LoopNarrationDirector.PlaySection` 由作者的关卡流程调用。

建议台本记录这些字段：`段落 ID`、`中文`、`英文`、`出现条件`、`结束条件/秒数`、`结束后事件`。条件驱动的句子可使用 `Duration = 0`，达到结束条件后再调用下一句或 `Clear Subtitle`。

## 左右分区菜单示例

独立示例：`/Game/MegaJam2026/UI/Menus/WBP_MainMenuSplitExample`。

左侧约 58% 展示实时的草地、长椅和路灯；右侧为深色菜单区，包含 Loop 标题、右下方莫比乌斯环和四个菜单按钮。菜单保留设置页的打开/返回行为。

示例使用 `Camera_OpeningUI_SplitExample`，标签为 `OpeningUISplitCamera`；该标签设置在示例蓝图的 `Opening Camera Tag` 类默认值中。调用现有 `Show Main Menu` 时将 `Menu Class` 设为这个示例即可使用。原菜单继续使用 `OpeningUICamera`。
