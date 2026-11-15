import json
import math
from pathlib import Path

import unreal


ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LEVELS = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
EDITOR = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
ROOT = '/Game/MegaJam2026'
CUBE = unreal.load_asset('/Engine/BasicShapes/Cube')
REPORT = []


def material(name, color):
    path = ROOT + '/Materials/Instances/' + name
    asset = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if not asset:
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, ROOT + '/Materials/Instances', unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew())
        unreal.MaterialEditingLibrary.set_material_instance_parent(
            asset, unreal.load_asset('/Engine/BasicShapes/BasicShapeMaterial'))
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            asset, 'Color', unreal.LinearColor(*color, 1.0))
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


LIGHT = material('MI_HalfMobius_Light', (0.62, 0.62, 0.62))
PATH = material('MI_HalfMobius_Path', (0.38, 0.38, 0.38))
DARK = material('MI_HalfMobius_Dark', (0.11, 0.11, 0.11))


def register(actor, label, folder, tags):
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    actor.set_editor_property('tags', list(actor.tags) + tags)
    REPORT.append(dict(label=label, path=actor.get_path_name(), cls=actor.get_class().get_name()))
    return actor


def cube(label, position, size, folder, tags, surface=LIGHT, yaw=0.0):
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*position),
                                         unreal.Rotator(pitch=0, yaw=yaw, roll=180 if 'HalfMobius_Upper' in tags else 0))
    register(actor, label, folder, tags)
    component = actor.static_mesh_component
    component.set_static_mesh(CUBE)
    component.set_material(0, surface)
    component.set_collision_profile_name('BlockAll')
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_editor_property('generate_overlap_events', False)
    component.set_mobility(unreal.ComponentMobility.STATIC)
    actor.set_actor_scale3d(unreal.Vector(*(value / 100.0 for value in size)))
    return actor


def blueprint(name, label, position, folder, tags, yaw=0.0, inverted=False, properties=None):
    actor_class = unreal.EditorAssetLibrary.load_blueprint_class(ROOT + '/Blueprints/Actors/BP_' + name)
    assert actor_class, name
    actor = ACTORS.spawn_actor_from_class(actor_class, unreal.Vector(*position),
                                         unreal.Rotator(pitch=0, yaw=yaw, roll=180 if inverted else 0))
    register(actor, label, folder, tags)
    for key, value in (properties or {}).items():
        actor.set_editor_property(key, value)
    actor.set_actor_transform(actor.get_actor_transform(), False, True)
    return actor


def sign(label, text, position, folder, tags, yaw=180.0, inverted=False, size=48.0):
    actor = ACTORS.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(*position),
                                         unreal.Rotator(pitch=0, yaw=yaw, roll=180 if inverted else 0))
    component = actor.text_render
    component.set_text(text)
    component.set_world_size(size)
    component.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
    component.set_text_render_color(unreal.Color(220, 220, 220, 255))
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    return register(actor, label, folder, tags)


def pressure(name, label, position, folder, tags, inverted=False, properties=None):
    settings = dict(show_debug_messages=False)
    settings.update(properties or {})
    return blueprint('PressurePlate' + name, label, position, folder, tags,
                     inverted=inverted, properties=settings)


def build_main():
    assert EDITOR.get_editor_world().get_path_name().startswith(ROOT + '/Maps/Gameplay/L_Main.')
    tag = 'HalfMobius_Phase1'
    assert not any(tag in [str(value) for value in actor.tags] for actor in ACTORS.get_all_level_actors()), 'Blockout already exists'
    samples = 33
    radius = 5500.0
    heights = [180, 210, 240, 270, 300, 270, 240, 210, 180, 210, 240,
               270, 300, 330, 300, 270, 240, 210, 180, 210, 240, 270,
               300, 330, 360, 330, 300, 270, 240, 210, 180, 180, 180]
    assert len(heights) == samples
    route = []
    for index in range(samples):
        angle = -math.pi / 2 + math.pi * index / (samples - 1)
        route.append((radius * math.cos(angle), radius * math.sin(angle), math.degrees(angle) + 90))
    for layer, direction in [('Lower', 1), ('Upper', -1)]:
        folder = 'Main/HalfMobius/' + layer
        layer_tags = [tag, 'HalfMobius_' + layer]
        for index, (center_x, center_y, yaw) in enumerate(route):
            angle = math.radians(yaw - 90)
            for lane in [-1, 0, 1]:
                if lane and index % 5 == (1 if lane < 0 else 3):
                    continue
                offset = lane * 400.0
                center = (center_x + math.cos(angle) * offset,
                          center_y + math.sin(angle) * offset)
                height = heights[index] + (0 if lane == 0 else (45 if (index + lane) % 3 else -30))
                thickness = height + 40.0
                center_z = direction * (-20000.0 + height - thickness / 2.0)
                width = 450.0 if lane == 0 else 400.0
                cube('HM_' + layer + '_Island_' + str(index).zfill(2) + '_Lane_' + str(lane),
                     (*center, center_z), (width, 370.0, thickness), folder, layer_tags,
                     PATH if lane == 0 else LIGHT, yaw)
        for index in [0, samples - 1]:
            center_x, center_y, yaw = route[index]
            height = heights[index]
            cube('HM_' + layer + '_Hub_' + str(index),
                 (center_x, center_y, direction * (-20000 + height - 120)),
                 (1400, 1500, 240), folder + '/Hubs', layer_tags, LIGHT)
        for index in range(8):
            angle = -math.pi / 2 + math.pi * (index + 0.5) / 8
            distance = radius + 1900 + 200 * (index % 3)
            height = [700, 1200, 900, 1600][index % 4]
            cube('HM_' + layer + '_Landmark_' + str(index),
                 (distance * math.cos(angle), distance * math.sin(angle),
                  direction * (-20000 + height / 2 - 20)),
                 (260 + 60 * (index % 2), 320, height), folder + '/Landmarks',
                 layer_tags + ['HalfMobius_Landmark'], DARK, math.degrees(angle))
    lower_folder = 'Main/HalfMobius/Lower/Gameplay'
    upper_folder = 'Main/HalfMobius/Upper/Gameplay'
    tags = [tag, 'HalfMobius_Gameplay']
    end_x, end_y, unused_yaw = route[-1]
    lower_surface = -20000 + heights[-1]
    pressure('Gravity', 'HM_Gravity_ToUpper', (end_x, end_y, lower_surface + 50),
             lower_folder, tags, properties=dict(flip_mode=unreal.GravityPlateMode.FORCE_UP))
    sign('HM_Sign_Gravity', 'FLIP / UPPER WORLD', (end_x + 440, end_y + 450, lower_surface + 210),
         lower_folder, tags, yaw=-90, size=42)
    start_x, start_y, unused_yaw = route[0]
    goal_surface = 20000 - heights[0]
    pressure('Goal', 'HM_Goal_Phase1End', (start_x, start_y, goal_surface - 50),
             upper_folder, tags, inverted=True,
             properties=dict(goal_tag='HalfMobius_Phase1End', lock_player_input_on_reach=False))
    sign('HM_Sign_Phase1End', 'PHASE 1 END / SECOND HALF RESERVED',
         (start_x + 450, start_y - 450, goal_surface - 220), upper_folder, tags,
         yaw=90, inverted=True, size=34)
    launch_index = 21
    center_x, center_y, yaw = route[launch_index]
    angle = math.radians(yaw - 90)
    pressure('Launch', 'HM_Launch_Optional',
             (center_x + 400 * math.cos(angle), center_y + 400 * math.sin(angle),
              -20000 + heights[launch_index] + 45 + 50), lower_folder, tags)
    lift_index = 8
    center_x, center_y, yaw = route[lift_index]
    angle = math.radians(yaw - 90)
    plate_x, plate_y = center_x - 400 * math.cos(angle), center_y - 400 * math.sin(angle)
    pressure('Lift', 'HM_Lift_OptionalBranch', (plate_x, plate_y, -20000 + heights[lift_index] + 45 + 50),
             lower_folder, tags, properties=dict(lift_region_tag='HalfMobius_Lift_A'))
    blueprint('LiftRegionController', 'HM_LiftRegion_A', (plate_x, plate_y, -20000),
              lower_folder, tags, properties=dict(lift_region_tag='HalfMobius_Lift_A', auto_start=False))
    for index in range(4):
        distance = 900 + index * 450
        height = 225 + index * 30
        thickness = 700.0
        block = blueprint('GroundBlock', 'HM_RisingBranch_' + str(index),
                          (center_x - distance * math.cos(angle), center_y - distance * math.sin(angle),
                           -20000 + height - thickness / 2 - 600), lower_folder, tags,
                          properties=dict(lift_region_tag='HalfMobius_Lift_A', lift_height=600.0, lift_duration=2.0))
        block.set_actor_scale3d(unreal.Vector(3.8, 3.8, thickness / 100))
        block.get_component_by_class(unreal.StaticMeshComponent).set_material(0, LIGHT)
    start = ACTORS.spawn_actor_from_class(unreal.PlayerStart,
                                         unreal.Vector(start_x - 400, start_y, -20000 + heights[0] + 110),
                                         unreal.Rotator(pitch=0, yaw=0, roll=0))
    register(start, 'PlayerStart_MainHalfMobius', 'Main/Gameplay', [tag, 'HalfMobius_Start'])
    sign('HM_Sign_Start', 'HALF MOBIUS / 01\nISLANDS > LIFT > LAUNCH > FLIP',
         (start_x + 500, start_y - 450, -20000 + heights[0] + 240),
         lower_folder, tags, yaw=180, size=36)
    EDITOR.set_level_viewport_camera_info(unreal.Vector(14000, -14000, -11000), unreal.Rotator(pitch=-30, yaw=135, roll=0))
    return dict(map=world_path(), samples=samples, radius_cm=radius, lower_route_cm=math.pi * radius,
                start=[start_x - 400, start_y, -20000 + heights[0] + 110],
                topology='Discrete lower semicircle, gravity transfer, reversed upper semicircle; second spatial half intentionally empty. Not a continuously twisted walkable surface.')


def world_path():
    return EDITOR.get_editor_world().get_path_name()


def build_pressure_test():
    assert world_path().startswith(ROOT + '/Maps/Dev/MovementTest.')
    tag = 'PressurePlateTest_v1'
    assert not any(tag in [str(value) for value in actor.tags] for actor in ACTORS.get_all_level_actors()), 'Test area already exists'
    folder = 'Tests/PressurePlates'
    tags = [tag]
    cube('PT_TestDeck', (5200, 0, -30), (5400, 4400, 60), folder, tags, LIGHT)
    sign('PT_Sign_Entrance', 'PRESSURE PLATE TEST\nLIFT / LAUNCH / GOAL / GRAVITY',
         (2900, -2050, 220), folder, tags, yaw=180, size=42)
    stations = [('Lift', -1500, '01 / LIFT / ONE SHOT'),
                ('Launch', -500, '02 / LAUNCH / REUSABLE'),
                ('Goal', 500, '03 / GOAL / NO LEVEL CHANGE'),
                ('Gravity', 1500, '04 / FLIP UP / RETURN ABOVE')]
    for name, station_y, text in stations:
        properties = {}
        if name == 'Lift':
            properties['lift_region_tag'] = 'PressureTest_Lift_A'
        elif name == 'Gravity':
            properties['flip_mode'] = unreal.GravityPlateMode.FORCE_UP
        elif name == 'Goal':
            properties.update(goal_tag='PressureTest_Goal', lock_player_input_on_reach=False)
        pressure(name, 'PT_' + name, (3600, station_y, 50), folder, tags, properties=properties)
        sign('PT_Sign_' + name, text, (4050, station_y - 260, 200), folder, tags, size=36)
    blueprint('LiftRegionController', 'PT_LiftRegion_A', (4600, -1500, 0), folder, tags,
              properties=dict(lift_region_tag='PressureTest_Lift_A', auto_start=False))
    for index, height in enumerate([100, 250, 400]):
        block = blueprint('GroundBlock', 'PT_LiftBlock_' + str(height) + 'cm',
                          (5000 + index * 650, -1500, height - 400 - 600), folder, tags,
                          properties=dict(lift_region_tag='PressureTest_Lift_A', lift_height=600.0, lift_duration=2.0))
        block.set_actor_scale3d(unreal.Vector(4, 4, 8))
        block.get_component_by_class(unreal.StaticMeshComponent).set_material(0, PATH)
    cube('PT_LaunchLanding', (5200, -500, 300), (1400, 900, 600), folder, tags, PATH)
    pressure('Gravity', 'PT_Gravity_ReturnDown', (4450, 1500, 19830), folder, tags, inverted=True,
             properties=dict(flip_mode=unreal.GravityPlateMode.FORCE_DOWN))
    sign('PT_Sign_Return', 'RETURN / FLIP DOWN', (4750, 1500, 19650), folder, tags,
         yaw=180, inverted=True, size=40)
    EDITOR.set_level_viewport_camera_info(unreal.Vector(1600, -4300, 3000), unreal.Rotator(pitch=-28, yaw=42, roll=0))
    return dict(map=world_path(), entrance=[2500, 0, 0], plates={name:[3600, station_y, 50] for name, station_y, text in stations},
                return_plate=[4450, 1500, 19830], region='PressureTest_Lift_A', original_player_start_preserved=True)


TARGET = globals().get('BLOCKOUT_TARGET', 'main')
assert TARGET in ['main', 'pressure_test'], TARGET
SUMMARY = build_main() if TARGET == 'main' else build_pressure_test()
assert LEVELS.save_current_level()
SUMMARY['created_count'] = len(REPORT)
SUMMARY['actors'] = REPORT
Path(unreal.Paths.project_saved_dir(), 'half_mobius_' + TARGET + '_report.json').write_text(
    json.dumps(SUMMARY, ensure_ascii=False, indent=2), encoding='utf-8')
unreal.log('HALF_MOBIUS_BUILT '+TARGET+' '+str(len(REPORT)))
