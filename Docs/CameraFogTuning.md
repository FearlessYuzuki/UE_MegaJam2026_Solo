# Camera visibility and fog

## Scope

`APlayerCharacter::BeginPlay` registers the fog view extension. It only changes perspective player views in worlds whose World Settings actor has the `LimitedFogView` tag. `L_Tutorial` has this tag. Editor cameras, scene captures, and untagged maps retain their original projection.

## Visibility

- `mj.FogViewDistance` is the perspective far clipping plane in centimeters. The default is `18000` (180 m), less than the approximately 190 m from either water surface to the nearest middle fog card.
- `mj.FogViewDistance 0` disables the override for comparison.
- The finite reversed-Z projection updates both clipping and the view frustum. The limit is forward camera-space depth, not a spherical streaming radius.
- This limits the main player view, not actor ticking, physics, streaming, shadow rendering, or all Lumen work. Large intersecting primitives can still be submitted.

## Surrounding mist

`HeightFog_Subtle` uses analytic Exponential Height Fog rather than the previously enabled 500 m volumetric fog. It starts 2 m from the camera, uses nearly uniform height density (`0.22`, height falloff `0.00001`), and stops integrating at 180 m. RGB fog luminance is neutral `(1000, 1000, 1000)` to match the level fixed exposure EV100 11.5; normalized low values appeared black at this exposure. The four low-density EasyFog cards remain as the middle band.

The analytic distance fog obscures the approach to the clipping plane; it does not replace the actual projection limit. If the clip distance changes, retune the fog integration distance and density together.

## Motion blur

At BeginPlay the first-person camera overrides Motion Blur Amount to `0.15`, Max to `1.0` (percent screen width), and Target FPS to `60`, with post-process blend weight `1`. This adds a small rendering cost and is not an FPS optimization. The motion blur scalability setting must remain enabled.

## Validation

The Win64 Development Editor build succeeded and hot reloaded. In PIE, with fog temporarily disabled, an 80 m test cube remained visible and a 220 m test cube disappeared at the default clip distance. Setting the console variable to zero restored both cubes. The console variable was restored to 18000, PIE was stopped, and both temporary test actors were removed. Camera post-process values were read back in PIE. FPS improvement and fast-turn motion blur comfort have not been benchmarked.
