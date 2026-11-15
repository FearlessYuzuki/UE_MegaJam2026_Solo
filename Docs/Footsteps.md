# Footsteps

The native `FootstepAudio` component is inherited by `BP_PlayerCharacter`. Its sound arrays, volume, step distance, and minimum interval are editable in the Blueprint component details. Defaults are Grass/Concrete volume 1.2, separate Water volume 2.0, step distance 165 cm, and minimum interval 0.18 s. Crouching reduces the selected surface volume to 65%. Jump and landing gains apply to that same surface volume.

## Jumping, landing, and water mix

Actual character jumps emit a quieter push-off using the previous grounded surface. `LandedDelegate` selects the destination surface from the landing hit, including inverted gravity. Impacts below 180 cm/s are ignored; louder landing gain scales from 1.1 to 1.6 with impact speed. Jump pitch is 1.08 and landing pitch is 0.9, with the existing small random variation. These reuse the verified surface recordings rather than claim separate landing recordings. Landing resets walking cadence to prevent an immediate duplicate footstep.

Water remains the main sound. Each water step/jump/landing adds a Concrete layer at 18% of the main voice gain, low-passed at 1800 Hz; the main water voice uses 6000 Hz. Dry Grass and Concrete events do not add these layers or water processing. Sounds play as local first-person audio and end on world cleanup.

`/Game/MegaJam2026/Audio/Footsteps/SM_WaterFootstepReverb` is a new dedicated submix using UE's built-in `SubmixEffectReverbPreset`. Only the water main voice sends to it, at 0.25. It has zero dry output, 0.8 s decay, HF gain 0.25, and HF decay ratio 0.35, producing a short damped tail without changing other game audio. No existing project reverb preset was found; the underwater Movement SoundWaves were verified to exist but are not used as substitutes for actual DSP reverb.

## Verified source recordings

Unreal Engine MCP was queried before integration. The asset registry contains 24 Water, 46 Grass, and 27 Concrete SoundWaves under `/Game/UltimateRealismFootsteps`. Each category currently uses recordings 1–8:

- `/Game/UltimateRealismFootsteps/Water/WAV/Water1` through `Water8`.
- `/Game/UltimateRealismFootsteps/Grass/WAV/Grass1` through `Grass8`.
- `/Game/UltimateRealismFootsteps/Concrete/WAV/Concrete1` through `Concrete8`.

All 24 selected recordings loaded as nonlooping SoundWaves with positive durations, exported successfully as WAV, and contained nonzero PCM samples. Existing `GameReady` Sound Cues had zero duration and missing references to `/Game/UltimateRealismFootstepsCollection/...`; this component directly references the valid SoundWaves. It randomly selects a recording without immediate repetition and varies pitch from 0.96 to 1.04.

## Surface selection

The component uses the character movement floor hit, including inverted gravity, rather than a world-down trace. Walking distance determines cadence. Stationary characters, airborne movement, input-locked gravity transitions, and large teleports do not emit steps.

Actor or floor-component tags override automatic classification: `Footstep.Water`, `Footstep.Grass`, or `Footstep.Concrete`.

Water planes in `L_Tutorial` have no collision; the character stands on the bed below them. At BeginPlay, the component caches water meshes using the existing FlipWorld deep-water materials, Advanced Water material, or the water override tag. It checks visible water plane bounds and whether the feet are within 35 cm of the surface, with its normal opposing current gravity. Raised blocks above that range use their own surface.

Ground Textures soil materials and UltraStylizedGrass ground materials use Grass sounds, including SOIL06 before activation and SOIL10 after activation. Activated `GroundBlock` actors also use Grass sounds because their walkable top and grass instances have no collision. Other blocks default to Concrete.

Material instances are classified through their parent chain, so runtime tinting and material swaps retain surface recognition.

Water meshes dynamically spawned after BeginPlay are not included in the cached plane list. A colliding water surface can still be classified from its material or override tag.

## Validation

The Editor development build passed. PIE verification in `L_Tutorial` tested walking on Water, Grass, and Concrete with both downward and upward gravity. All six cases emitted three correctly classified steps; all 18 observed AudioComponents were actively playing the matching SoundWave, without immediate recording repetition. The inverted Grass case used a dynamic material instance. Stationary and airborne checks emitted no additional steps. Test material changes were limited to the PIE world and discarded when PIE stopped.

Reports are in `Saved/footstep_runtime_verification.json`, `Saved/footstep_assets_inspection.json`, and `Saved/footstep_pcm_verification.json`.

The jump/water follow-up build also passed. All six surface/gravity combinations emitted both actual jump and landing events with matching recordings, and stationary landing did not add duplicate walking events. Water low-pass values and the 0.18 ground-layer gain were checked on playing components. Recording the dedicated wet-only submix while walking produced nonzero PCM output, confirming that reverb routing and processing execute. UE's background-audio preference was temporarily enabled for this recording and restored afterward. Follow-up reports are `Saved/jump_water_audio_verification.json`, `Saved/water_reverb_pcm_verification.json`, and `Saved/water_plate_audio_asset_inspection.json`.
