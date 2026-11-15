# Tutorial outdoor set and rain ambience

## Tutorial GroundBlock

`/Game/MegaJam2026/Maps/Gameplay/L_Tutorial` contains the user's existing `BP_GroundBlock` at `(-79130, -49320, -20010)`, retaining its original transform and 4 m square footprint.

- Initial material: `/Game/Ground_Textures_v1/Materials/M_Ground_v1_06`.
- Activated sides: `/Game/Ground_Textures_v1/Materials/M_Ground_v1_10`.
- Grass top: `/Game/UltraStylizedGrass/Materials/Ground/MI_Ground_Plane_NoVariation`.
- Grass mesh 03: `/Game/PN_FoliageCollection/Meshes/grassMesh/lowGrass_01_01_SM`.
- Grass spacing: 24 cm, scale 1, 289 instances covering the top.

The instance has **Start Activated** enabled, so its soil, grass and walking collision are ready in the editor and at BeginPlay. This option defaults to false on other GroundBlocks, preserving their pressure plate lift and landing activation behavior. A block that starts activated does not need to lift before it becomes solid.

The `Footstep.Grass` tag selects grass footsteps on this block, which sits only 10 cm above the water plane and would otherwise fall within the water footstep proximity threshold.

Outdoor assets are from the verified installed `Suburban_Household/VOL6_Outdoor` pack:

- `Outdoor_ParkBench`: `Meshes/LP/SM_Bench_Park_01a`.
- `Outdoor_StreetLamp`: `Meshes/LP/SM_Light_Post_Set_01a`.
- `Outdoor_StreetLamp_Light`: a movable warm point light, 24000 lumens, 700 cm radius. The lamp is scaled to 1.65 and uses `MI_OpeningParkLamp` with exponent 1 and a warm emissive multiplier; see `ResourcesNarrationAndAudio.md` for the material correction.

Both props are attached to the GroundBlock, and the light is attached to the lamp. Props retain normal world scale despite the scaled parent. They sit on the grass surface with separate footprints inside the block boundary. Actors are grouped under `Outdoor Set` in the Outliner.

## Shared rain audio

Both `L_Tutorial` and `L_Main` contain one native `RainAmbience` actor under the `Audio` folder, referencing their existing lower/upper rain emitters. Each actor plays the same nonspatial ambience at volume **0.4**, keeping sound consistent through gravity changes without doubling it for two world surfaces.

Project cue: `/Game/MegaJam2026/Audio/Ambience/SC_RainAmbience_Loop`.

Source: the existing `/Game/Water_Sounds/WAV/Rain/Rain_Moderate_1`, 68 seconds, stereo, 44.1 kHz. The project cue is duplicated from its original pack cue with wave-player looping enabled; source pack assets remain unchanged. Source export contains nonzero PCM.

The ambience fades in or out over approximately 0.5 seconds. It plays while at least one referenced rain emitter is visible, active, and has not been deactivated. Hiding or deactivating all referenced emitters silences it. Adjust `Volume` or `Rain Sound` on the map actor to tune the mix.

The formal level's platform layout is preserved; its only new actor is the rain ambience. Rain particles and tutorial brightness/fog settings are unchanged.
