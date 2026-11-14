# Camera visibility and fog

## Scope

`APlayerCharacter::BeginPlay` registers the fog view extension. It only changes perspective player views in worlds whose World Settings actor has the `LimitedFogView` tag. `L_Tutorial` has this tag. Editor cameras, scene captures, and untagged maps retain their original projection.

## Visibility

- `mj.FogViewDistance` is the perspective far clipping plane in centimeters. The default is `25000` (250 m).
- `mj.FogViewDistance 0` disables the override for comparison.
- The finite reversed-Z projection updates both clipping and the view frustum. The limit is forward camera-space depth, not a spherical streaming radius.
- This limits the main player view, not actor ticking, physics, streaming, shadow rendering, or all Lumen work. Large intersecting primitives can still be submitted.

## Surrounding mist

`HeightFog_Subtle` uses analytic Exponential Height Fog rather than the previously enabled 500 m volumetric fog. It starts 2 m from the camera, uses nearly uniform height density (`0.495`, increased by 50% from `0.33`, height falloff `0.00001`), and stops integrating at 250 m. RGB fog luminance is neutral `(1000, 1000, 1000)`. The fixed exposure is EV100 11.5 with exposure compensation -1 EV.

The four middle EasyFog cards have the `TransitFog` actor tag. The view extension always adds their primitive components to `HiddenPrimitives` for tagged perspective game views, including during gravity transitions. This is independent of `mj.FogViewDistance`, so disabling the far clip override does not reveal the cards. Editor views and scene/reflection captures are unchanged. The actors and materials remain in the level, and their zero tick-rate configuration is preserved; this is view-specific hiding, not removal or a guarantee of zero indirect rendering cost. Tagged actors are collected when the extension first encounters a world; dynamic spawning or retagging after that requires a world reload.

The analytic distance fog obscures the approach to the clipping plane; it does not replace the actual projection limit. If the clip distance changes, retune the fog integration distance and density together.

## Motion blur

At BeginPlay the first-person camera overrides Motion Blur Amount to `0.15`, Max to `1.0` (percent screen width), and Target FPS to `60`, with post-process blend weight `1`. This adds a small rendering cost and is not an FPS optimization. The motion blur scalability setting must remain enabled.

## Validation

Earlier 80 m / 220 m cube observations were inconclusive and are not evidence that the final far clip works correctly. Prior PIE logs confirmed that the extension was applied, and camera post-process settings were read back. The current 250 m clip and unconditional `TransitFog` game-view hiding still require final in-game visual validation, including looking upward and traversing the middle band. No FPS improvement is claimed.
