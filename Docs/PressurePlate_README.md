# 压力板系统（Pressure Plates）

四个压力板全部继承 `AWorldsBasicActor`（即场景里的 BasicBox），直接复用它的
`BaseBox` / `TriggerBox` 组件、玩家重叠检测和 `OnPlayerOverlapChanged` 广播，
不重复实现一套检测逻辑。

**编译状态**：`Build.bat MegaJam_2026Editor Win64 Development` → `Result: Succeeded`（UE 5.8.4）。

---

## 一、四个压力板

| 类 | 外观 | 需求 | 关键参数 |
|---|---|---|---|
| `APressurePlateGravity` | 紫色 · 方形 | 踩下翻转玩家重力，送到上方世界 | `FlipMode`（取反/强制上/强制下）、`bStopMovementOnFlip` |
| `APressurePlateLift` | 红色 · 圆形 | 控制地面地块升起 | `LiftRegionTag`、`bFallbackToGroundBlocks` |
| `APressurePlateGoal` | 黄色 · 方形 | 最后关卡完成 | `GoalTag`、`VictoryMessage`、`bLockPlayerInputOnReach` |
| `APressurePlateLaunch` | 绿色 · 方形 | 把玩家向上弹射 | `LaunchSpeed`、`LateralVelocityRetain`、`bForceFalling` |

公共行为（`APressurePlateBase`）：

- 板子会下沉 `PressDepth` 厘米，`PressDuration` 内回弹
- `bOneShot = true`（重力 / 升地块 / 目标三个默认）：触发后锁存、保持按下外观，不再响应，
  需要重新可用时调 `ResetPlate()`
- `bOneShot = false`（弹射板默认）：玩家被弹飞 → 离开触发盒 → 板子自动回弹 → **可以反复踩**
- 每次"进入"只触发一次，站在板上不会持续施加效果（弹射板不会一直往上顶）
- 事件：`OnPlatePressed` / `OnPlateReleased`（C++ 和蓝图都能接）、`BP_OnPlatePressed` / `BP_OnPlateReleased`（纯蓝图事件）
- `PressSound` / `ReleaseSound` 各播一次
- 触发盒自动跟随 `PlateSize`，水平每边比板子多 `TriggerPadding`（默认 15cm），避免踩边缘踩空

### 高饱和度荧光配色

颜色定义在各自构造函数里，使用线性 RGB；已同步蓝图默认值与三张地图实例。按下时保持同一色相，亮度降至 65%。

| 板子 | 常态线性 RGB | 按下 |
|---|---|---|
| 紫 · 重力方形 | (0.65, 0.008, 1.0) | 常态 × 0.65 |
| 红 · 升降圆形 | (1.0, 0.008, 0.025) | 常态 × 0.65 |
| 黄 · 目标方形 | (1.0, 0.8, 0.005) | 常态 × 0.65 |
| 绿 · 弹射方形 | (0.025, 1.0, 0.008) | 常态 × 0.65 |

染色走材质里的 `Color` 参数（`PlateColorParameter`），默认使用项目材质
`/Game/MegaJam2026/Materials/M_PressurePlateGlow`，保留原来的颜色参数接口。自己换了材质的话，
把参数名改成材质里对应的那个即可。自定义板面材质请在压力板蓝图的类默认组件 `PlateMesh` 上设置；动态材质从这份可保存的模板材质创建，不从关卡重载后可能失去父材质的 MID 再套一层 MID。

### 黑白环境中的板面保色

- 仅 `PlateMesh` 开启 `Render CustomDepth Pass`，`CustomDepth Stencil Value = 1`；底座不启用保色标记。
- 复用已有 `M_PP_MonochromeSelective`：Stencil=1 且通过 CustomDepth / SceneDepth 可见性判断的像素保留原色，其余像素仍去色，不是透视显示被遮挡的板子。
- 游玩过程中使用上表荧光配色；底座与普通地块保持灰白。最终目标板可启用 `Restore World Color On Reach`，详见 `WorldColorAndMarkers.md`。
- 已同步四个 `BP_PressurePlate*` 类默认组件与 `L_Main`、`MovementTest` 中现有实例，并检查了保存重载后的 Stencil 和材质父链。
- `ApplyPlateColor()` 从蓝图类默认组件读取稳定源材质；只在 MID 不存在或父材质不匹配时重新创建，普通踩下/回弹继续复用 MID。

### 荧光

- 四种压力板统一使用 `M_PressurePlateGlow`：不透明 `Unlit` 自发光材质，不依赖场景灯光照亮板面。
- `Emissive Color = EyeAdaptationInverse(Color × GlowStrength)`，补偿场景曝光；避免强光场景中普通低数值自发光几乎不可见，也不需要用极高屏幕亮度制造光晕。
- `GlowStrength` 默认 `3.0`，运行时未按下/按下分别设为 `3.0/1.8`；`Color` 同步变暗，不新增 Tick、灯光或粒子。
- 调整运行时强度需修改 `ApplyPlateColor()` 的写入值；自定义板面源材质仍在压力板蓝图默认组件 `PlateMesh` 上配置。
- 不修改全场 Bloom、曝光、雾和黑白后处理。光晕大小受当前 Bloom 设置与曝光影响；Stencil 保色仅覆盖板面，扩散到板外的 Bloom 仍经过黑白处理，不是彩色透视描边。

---

## 二、和 BasicBox 的配合方式

压力板**不改** `AWorldsBasicActor` 的任何逻辑，只用它的扩展点：

```cpp
// 1. 订阅 BasicBox 已有的玩家重叠广播
OnPlayerOverlapChanged.AddUniqueDynamic(this, &APressurePlateBase::HandlePlateOverlap);

// 2. 关掉 BasicBox 原本"一踩上去就翻重力"的激进行为，
//    让重力板自己决定翻哪一次（受 FlipMode 控制）
virtual bool ShouldFlipPlayerOnOverlap() const override { return false; }

// 3. 构造函数里关掉会污染性能的默认项
bUseOverlapTint = false;      // 不再每次重叠 new 一个 MID
bShowDebugMessages = false;   // 不在 Shipping 里刷屏
```

重力翻转复用的还是原班设施，和 `AWorldsBasicActor::OnTriggerBoxBeginOverlap` 里完全一致的两步：

```cpp
Movement->SetGravityDirection(TargetGravity);   // USourceCharacterMovementComponent
Player->SetCameraAngle();                       // 180° 镜头翻滚 + 输入锁
```

地块升起复用的是 `ALiftRegionController::RequestLift()` → 广播 → `AGroundBlock::StartLift()`。
为此给 `ALiftRegionController` 加了一个只读 getter（纯加法，不改行为）：

```cpp
UFUNCTION(BlueprintPure, Category = "Lift Region")
FName GetLiftRegionTag() const { return LiftRegionTag; }
```

找不到控制器时会退化为直接遍历同 Tag 的 `AGroundBlock` 调 `StartLift()`，保证踩了不会没反应。

### 场景接线

- **升地块板**：场景放一个 `ALiftRegionController`，`LiftRegionTag = "LiftRegion_A"`；
  要升的地块 `AGroundBlock` 的 `LiftRegionTag` 设成同一个值；压力板默认也是这个 Tag。
- **目标板**：蓝图里接 `OnGoalReached` → 打开结算 UI / 切关卡 / 停表。
- **弹射板**：`LaunchSpeed` 默认 1200（玩家跳跃初速约 420），想要更夸张就往上调。
  可反复踩，弹飞后离开触发盒自动回弹，落回来再踩就再弹一次。
- **重力板**：零连线，踩下即翻。想加音效/特效接 `OnPlatePressed` 或 `OnGravityFlipped`。

### 复用

**弹射板（反复踩）**：`bOneShot = false`。玩家被弹飞、离开触发盒的那一刻板子自动回弹，
落回来再踩一次就再弹一次。不需要任何外部调用。

**另外三块（一次性）**：`bOneShot = true`，触发后锁存并保持按下外观。重新可用要显式调
`ResetPlate()`（`BlueprintCallable`）：

```cpp
// 典型挂载点：重生 → 检查点 → 另一块压力板达成
PressurePlate->ResetPlate();
```

`bOneShot` 是 `EditAnywhere`，单块板子想换模式直接在细节面板勾，不用改代码。

### 重启编辑器

新增了 UCLASS，UHT 要重新生成反射代码。**必须关掉编辑器重新编译后再打开**——
Live Coding 不认识新类。（本次编译因为编辑器开着，产物是带编号的
`UnrealEditor-MegaJam_2026-0019.dll`，运行中的编辑器仍在用旧 DLL。）

---

## 三、蓝图 vs C++ 性能对比

### 先说结论

压力板是**事件驱动**的：一次按下只跑几条逻辑，不是每帧都跑。所以语言选择在帧预算里
几乎看不出来——但"写法"上的差异（有没有 Tick、有没有每次 new 对象）能差两个数量级。

下面的数字是**量级估算**，不是本机实测。

### 单次触发开销

| | 蓝图 | C++ | 倍数 |
|---|---|---|---|
| 一次按下（约 30–60 节点链路） | 5–20 µs | 0.1–0.5 µs | 10–50× |
| 占 60fps 帧预算（16.6ms） | ≈ 0.12% | ≈ 0.003% | — |

这点开销只在踩下去的瞬间发生一次。**玩家踩一次板子花 20µs，完全无所谓。**

### 逐帧开销才是分水岭

| | 蓝图 Timeline 方案 | 本实现的 C++ |
|---|---|---|
| 静止时 | Timeline 组件常驻，每帧评估曲线 | **Tick 完全不注册** |
| 按下动画期间 | 每帧 BP 曲线求值 + 设相对位置 | 每帧一次 float 插值 + `SetRelativeLocation` |
| 单实例 Tick | 20–80 µs | 0.5–2 µs |
| 100 块板子同时 Tick | 2–8 ms（吃掉半个帧） | 0.05–0.2 ms |
| 实例内存 | 每个 BP 实例带 EventGraph 执行上下文 | 只有成员变量 |

本实现的关键设计：`bStartWithTickEnabled = false`，只在 `PressDuration`（0.12s）内开 Tick，
动画结束立刻 `SetActorTickEnabled(false)`。所以 **100 块板子摆满关卡，静止时 CPU 成本是 0**。

### 比"用哪种语言"更影响性能的三件事

1. **每次重叠都 new 一个 MID** —— 这是你现有代码里的真问题。
   `AWorldsBasicActor::ApplyMaterialToBlock()` 每次 `BeginOverlap` 都
   `UMaterialInstanceDynamic::Create()`，产生 UObject 分配 + GC 压力。
   压力板里改成*只创建一次 MID，之后只 `SetVectorParameterValue`*。
   如果 BasicBox 上要频繁重叠，建议把这个也改成缓存版。

2. **`bShowDebugMessages` 默认 true** —— 每次重叠 `AddOnScreenDebugMessage`，
   Shipping 里也在跑。`AWorldsBasicActor` 的默认值建议改成 `false`，
   或者在外面包 `#if !UE_BUILD_SHIPPING`。

3. **`TActorIterator` 全场景遍历** —— 别放进 Tick 或每次触发里。
   我在 `BeginPlay` 里缓存一次（`CacheLiftControllers`），只有手动刷新才重扫。

### 实际建议：C++ 打底 + 蓝图调参

四个类都是 `Blueprintable`。最舒服的工作流是：

1. C++ 存放逻辑与默认值（已写好，编译通过）
2. 在 Content 里 `Create Blueprint Class` → 选这四个父类之一
3. 关卡里只摆蓝图子类，颜色/弹射速度/触发盒尺寸都在细节面板调
4. 需要额外表现（粒子、镜头抖、通关 UI）就在蓝图事件里接

热路径留在 C++，迭代留在蓝图。顺带一提：UE5 已经移除了蓝图 nativization
（BP 转 C++），所以别指望那个功能。

### 如果非要纯蓝图

等效节点链（每块板子约 25 个节点）：

```
Event ActorBeginOverlap (Box Collision)
  → Cast<PlayerCharacter> → Branch
  → Timeline (0→1, 0.12s, 输出 Float)
      → SetRelativeLocation (Plate.z = -4 * Float)
  → Play Sound at Location
  → 效果分叉（翻重力 / RequestLift / 通关 / SetVelocity）
Event ActorEndOverlap → Reverse Timeline
```

蓝图方案唯一真正的成本是 Timeline 组件常驻 + 每帧评估，以及上面说的 MID 分配。
一块板子无所谓，几十块就有差别了。

---

## 四、文件清单

```
Source/MegaJam_2026/Actors/
├── PressurePlateBase.h / .cpp      基类：状态机、动画、颜色、事件
├── PressurePlateGravity.h / .cpp   红·方形  重力翻转
├── PressurePlateLift.h / .cpp      红·圆形  地块升起
├── PressurePlateGoal.h / .cpp      黄·方形  关卡完成
├── PressurePlateLaunch.h / .cpp    绿·方形  向上弹射
└── LiftRegionController.h          +1 行 getter（additive）
```

## 五、还没验证的部分

编译与链接已通过，但**运行时行为没有在编辑器里跑过**（PIE 我这边起不来）。
第一次打开编辑器时建议按顺序确认：

1. 板子在地面上方、贴合 BaseBox 顶面（`bAutoAlignToBaseTop`）
2. 踩上去触发盒能否命中（踩边缘踩不到就调大 `TriggerPadding`）
3. 按下动画结束后 Actor 是否停止 Tick（`stat unit` / `stat game` 里看 Game 线程）
4. 重力板翻转后镜头是否正常 180° 翻滚、输入锁是否按时释放
5. 玩家重生后重力被 `PlayerCharacter.cpp:621` 重置为向下，但压力板仍是锁存状态——
   需要在重生逻辑里调一次 `ResetPlate()`

第 5 点是已知的语义缺口，属于设计取舍，不是 bug。
