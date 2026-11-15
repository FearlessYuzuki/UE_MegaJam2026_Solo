# Settings and Lumen

`WBP_MainMenuUI` derives from `ULoopMainMenuWidget`. Its Settings button opens the centered `SettingsPage` (`WBP_SettingsUI`), and Back/Escape returns to the menu. The panel uses a rounded Background Blur plus `M_UI_SettingsAcrylic`, a translucent UI material with a subtle gradient, grain and edge highlights. A Scale Box keeps the complete panel visible on smaller screens.

The settings page uses Noto Sans Regular for English text and Noto Sans CJK SC for Chinese text and translated dropdowns. Both fonts are imported into the project, with SIL Open Font License files in `Content/MegaJam2026/UI/Fonts/Licenses`. Main menu typography is unchanged. The dimmed main menu is disabled while Settings is open so keyboard focus stays out of the background controls.

To display the menu at runtime, call `ULoopMainMenuWidget::ShowMainMenu(PlayerController, WBP_MainMenuUI Class)`. It adds the menu to the viewport, shows the mouse cursor, and selects UI-only input. Existing menu spawning can continue to use the same widget asset.

Changes are drafts until **Apply settings**. Back/Escape discards unapplied changes on the next open. Reset defaults changes the draft; Apply saves it. Values persist in the generated `GameUserSettings.ini`, in `[/Script/MegaJam_2026.LoopSettings]`, alongside the standard UE user settings. A game instance subsystem reapplies them when the game starts.

| Control | Values | Default |
| --- | --- | --- |
| Mouse sensitivity | Integer 1–200%; slider and numeric input | 100% |
| Graphics quality | Low / Medium / High | High |
| Frame limit | 30 / 60 / 90 / 120 / 144 / 165 / 240 / Unlimited | 120 |
| Language | English / 中文 | English |

Mouse input is multiplied by the selected percentage in `APlayerCharacter::DoLook`. Keyboard input rejects non-ASCII digits; pasted text is sanitized. Values are clamped on commit and before applying. Frame limits are ceilings; VSync, hardware and scene cost can reduce the achieved FPS.

## Rendering

Low, Medium and High call UE `SetOverallScalabilityLevel(0/1/2)` respectively. These retain the engine's scalability presets, including their resolution quality. Only High enables both `r.Lumen.DiffuseIndirect.Allow` and `r.Lumen.Reflections.Allow`; Low and Medium force both off. The project keeps `r.DynamicGlobalIlluminationMethod=1`, `r.ReflectionMethod=1`, mesh distance fields, DX12/SM6 and hardware ray tracing support, so High can use Lumen without a restart. Hardware ray tracing support does not imply every machine uses hardware Lumen: the renderer chooses the supported path.

Low/Medium do not introduce baked GI; the project has static lighting disabled. Verify the level's lighting at these presets. Existing console overrides have higher priority than game settings and may override choices during development.

## Gameplay language integration

The language selector immediately previews Settings in English or Chinese, including labels, help text, quality options, Unlimited, Lumen status and save messages. Apply persists the selected language for Settings, narration and instructions; Back discards the draft. Main menu labels remain English, and the engine culture does not change. Dropdown values use stable indices so translated labels never change the saved quality or frame limit. Chinese text uses Noto Sans CJK SC. Use the **Loop Gameplay Language Subsystem** from a game instance for game content:

- `GetInstructionText(English, Chinese)` selects bilingual instruction/subtitle text, with English fallback when Chinese is empty.
- `GetNarrationSound(English, Chinese)` selects a sound asset with English fallback when the Chinese sound is missing.
- `PlayNarration(English, Chinese)` plays the selected sound and returns its audio component. A language change affects subsequent narration; existing audio continues.
- `OnGameplayLanguageChanged` lets other gameplay widgets refresh immediately.

`WBP_GameplayInstructionUI` derives from `ULoopInstructionWidget`. Set its `EnglishInstruction` and `ChineseInstruction`, or call `SetInstruction`; its `Text_Instruction` refreshes automatically when the language changes. Add this widget to the viewport from the gameplay UI when an instruction is needed.

The project currently contains no bilingual narration audio or authored instruction content. The reusable widget and selection/playback API provide the connection points; add both language versions when authoring that content.

## Editor previews

Open `/Game/MegaJam2026/UI/Previews/WP_MainMenu` to exercise Settings and Back, or `WP_Settings` for the standalone settings panel. The original main menu asset is copied once to `Saved/SettingsBackup/WBP_MainMenuUI.before-settings.uasset` before reparenting.

## Validation

The UE 5.8 Editor target compiles successfully. PIE checks cover Settings/Back navigation, integer sanitization and clamping, slider synchronization, all three scalability levels and both Lumen allow variables, finite/unlimited frame limits, INI persistence, cancellation of drafts, gameplay text language selection, empty-Chinese fallback, and unchanged English main menu labels. Keyboard-driven text input was also checked with letters, punctuation and Chinese characters: only the digits remain. Settings localization checks cover immediate Chinese preview and Noto Sans CJK fonts, translated quality/unlimited values, apply/reopen, canceled English drafts, reset defaults, and English main menu labels.
