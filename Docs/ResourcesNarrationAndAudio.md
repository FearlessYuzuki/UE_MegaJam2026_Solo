# Resources 台本与音乐

原始台本：`Content/Resources/Assets/Resources/Outro.txt`。

## 字幕内容

原台本已整理为 `/Game/MegaJam2026/Narrative/DT_LoopNarration`，共 12 段、52 句。中文保留原文，操作提示统一排版，流程括号说明保存在 `TriggerNote`。英文是对应字幕翻译。`Duration` 是按文字长度估算的初始阅读时间，可在 DataTable 中修改。

两关均有 `NarrationDirector_L_Tutorial` 或 `NarrationDirector_L_Main`，位于 Outliner 的 `Narrative` 文件夹。调用其 `Play Section`，传入下表中的 Section；当前句播放完会继续下一句，整段结束清除字幕并广播 `On Section Finished`。调用新的段落会替换当前段落，`Stop Narration` 可随时清除。

| Section | 台本触发说明 |
| --- | --- |
| `Tutorial.Intro` | 教学开始 |
| `Tutorial.GroundRaised` | 地块升起后，包含操作提示 |
| `Tutorial.PlateIntro` | 第 4 个踏板出现后 |
| `Tutorial.Lift` | 第一个升地块踏板触发 |
| `Tutorial.Launch` | 第二个弹射踏板触发 |
| `Tutorial.Flip` | 最高处第三个踏板触发并翻转后 |
| `Tutorial.Exit` | 翻转后，最后一个切关/结束踏板前 |
| `Main.Landing` | 主关卡从空中落地后 |
| `Main.Flip1` | 第一次反转后 |
| `Main.Flip2` | 第二次反转后 |
| `Main.Flip3` | 第三次反转后 |
| `Main.End` | 最后一次反转并弹射到空中收尾后 |

`Tutorial.Intro` 已在 NewGame 黑屏渐显、玩家开始下落后自动触发；其余段落由作者的关卡流程调用。原文中的“一模黑白”“苦难本身本身”“知道你给出新的答案”等字句保留，方便作者后续修订。没有新增或假定存在旁白录音。

## BGM：仅主关卡

`L_Main` 的 `BGM_Main_Playlist` 使用用户确认的四首顺序循环：

1. `kf011112-flexible` — 167.78 秒。
2. `kf025703-thumper` — 87.18 秒。
3. `kf025704-time-to-go-far` — 318.89 秒。
4. `kf031009-space-work` — 186.95 秒。

全部为 `/Game/Resources/Assets/Resources/InGameBGM/` 下已存在的 SoundWave。音量初设为 0.42，原录音不修改。播放由音频完成事件推进到下一首，第四首完成后回到第一首；`Stop Playlist` 明确停止，`Start Playlist` 从第一首重新开始。离开关卡后音频随关卡销毁。教学关卡没有 BGM Actor。

## 环境音

用户提供的 `Natual_Sound/atmo-coast-beach-waves-on-sand-01.wav` 原先仅为磁盘 WAV，已导入为同目录 SoundWave。两关均有 `Ambient_WaterCoast_*`，低音量 0.12 循环播放，与原有雨声独立混合。

五个原始 WAV 均已检查到非零 PCM；四首 BGM 的 UE 资产时长与原文件一致。音乐与环境音 Actor 的 `Volume` 和 `Playlist` 可直接在关卡细节面板修改。

## 左侧构图与路灯修复

`Camera_OpeningUI_SplitExample` 后移并调整朝向，为地块右侧留出完整边缘。原菜单构图和关卡中的地块位置不变。

资源包材质中的 `Emissive Power` 实际连接到 Power 节点的指数输入。提高该参数会使灯罩纹理的大部分像素趋近于黑，只剩少量亮点。专用 `MI_OpeningParkLamp` 已将指数恢复为 1，以 `Emissive Color Multi` 调整暖色发光，并保留独立的真实 PointLight。原资源包材质未修改。

### 灯罩展示时的闪烁修正

固定镜头连续帧对比确认高强度自发光会放大 Lumen 漫反射间接光的时域噪点；关闭反射或增加 TSR 历史分辨率没有同等改善。灯罩未启用闪烁动画，摄像机变换保持固定。

`MI_OpeningParkLamp` 改用项目专用父材质链 `Opening/MI_ParkLampStableBase` → `Opening/M_ParkLampStable`。独立父材质保留资源包的表面属性，在 RayTracingQualitySwitch 的 RayTraced 输入使用零自发光，减少 Lumen Surface Cache 与射线采样中的噪声贡献。屏幕追踪仍可能看到直接自发光，因此最终同时把可见发光倍率从 `(50000,26000,7000)` 调整到 `(15000,7800,2100)`，Roughness Multi 设为 3。路灯组件关闭动态间接光贡献、距离场光照贡献与 WPO，实际照明继续使用原有 24000 流明 PointLight。

最终方案在保持 Lumen、87% ScreenPercentage 与原 TSR 历史配置的测试中，灯罩区域 95 分位时域标准差由约 2.1 降至 0.62（8 帧、RGB 8 位）；超过 2 的波动像素由约 5% 降至约 0.006%。这是固定取景的像素波动测量，雨雾和草叶仍保留正常动态。全部诊断用渲染 CVar 已恢复，原资源包材质未修改。
