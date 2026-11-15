# Pressure plate audio candidates

Unreal Engine MCP loaded and exported these Engine SoundWave assets, and each preview contains nonzero PCM audio:

| Candidate | Engine asset | Duration | Suggested use |
| --- | --- | --- | --- |
| `VR_click2` | `/Engine/VREditor/Sounds/VR_click2` | 0.377 s | Short press |
| `VR_click1` | `/Engine/VREditor/Sounds/VR_click1` | 0.679 s | Alternative press |
| `VR_click3` | `/Engine/VREditor/Sounds/VR_click3` | 0.799 s | Alternative press |
| `VR_confirm` | `/Engine/VREditor/Sounds/VR_confirm` | 1.086 s | Goal confirmation |
| `VR_open` | `/Engine/VREditor/Sounds/VR_open` | 1.084 s | Lift activation |
| `VR_close` | `/Engine/VREditor/Sounds/VR_close` | 1.217 s | Release |
| `Click_on_Button` | `/Engine/VREditor/Sounds/UI/Click_on_Button` | 0.565 s | UI-like press |
| `Object_Snaps_To_Grid` | `/Engine/VREditor/Sounds/UI/Object_Snaps_To_Grid` | 1.145 s | Mechanical confirmation |

Selected: `VR_click2`, duplicated into `/Game/MegaJam2026/Audio/PressurePlates/S_PressurePlate_Click2`. `APressurePlateBase` uses this project SoundWave as the default `PressSound`; Gravity, Lift, Goal, and Launch Blueprint defaults were read back and confirmed to inherit it. `ReleaseSound` remains separately configurable. The selection does not require an Engine editor-content audio reference during gameplay.
