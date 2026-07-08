# Aura 工作区文件布局速查

这个文件用于帮助 AI 或新协作者快速定位本 Unreal Engine 项目的主要文件。系统架构和 GAS 细节请继续参考 `CLAUDE.md`，协作规则和构建命令请参考 `AGENTS.md`。

## 项目入口

- `Aura.uproject`: UE 5.3 项目文件，运行时模块名为 `Aura`。
- `Aura.sln`: Visual Studio 解决方案。
- `Aura.code-workspace`: VS Code 工作区。
- `Source/Aura/Aura.Build.cs`: C++ 模块依赖配置。当前主要依赖包括 `GameplayAbilities`、`GameplayTags`、`GameplayTasks`、`EnhancedInput`、`NavigationSystem`、`Niagara`、`AIModule`。
- `Config/Default*.ini`: 项目配置、输入配置、引擎配置、Gameplay Tags 配置。
- `Content/`: UE 资产、蓝图、地图、动画、材质、UI、特效和声音。
- `Data/`: 从曲线表导出的 JSON 快照，例如伤害和职业主属性数据。
- `Plugins/VisualStudioTools`: 编辑器集成插件，通常不要修改。

## 不要当作源码维护的目录

这些目录多为 UE 或 IDE 生成产物，通常不应作为人工修改目标：

- `Binaries/`
- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `.vs/`
- `.idea/`
- `packages/`

## C++ 源码布局

运行时 C++ 模块位于 `Source/Aura/`。`Public/` 和 `Private/` 基本按功能一一对应：新增 `.h` 放到 `Public/<Feature>/`，新增 `.cpp` 放到 `Private/<Feature>/`。

### `Source/Aura/Public` 与 `Source/Aura/Private`

- `AbilitySystem/`
  - GAS 核心代码。
  - `AuraAbilitySystemComponent`: 技能输入、技能激活和 effect tag 广播。
  - `AuraAttributeSet`: 属性、复制、元属性、伤害后处理。
  - `AuraAbilitySystemLibrary`: 常用 GAS 查询和辅助函数。
  - `AuraAbilitySystemGlobals`: 自定义 `GameplayEffectContext` 的入口。
  - `Abilities/`: 技能基类和具体技能，例如投射物、近战、召唤。
  - `AbilityTask/`: 自定义 AbilityTask，例如鼠标目标数据。
  - `Data/`: `UDataAsset` 类型，例如属性信息、职业信息、战斗插槽配置。
  - `ExecCalc/`: 执行计算，例如伤害公式。
  - `ModMagCalc/`: 属性派生计算，例如最大生命和最大法力。
- `Actor/`
  - 可放置或运行时生成的 Actor，例如 `AuraEffectActor`、`AuraProjectile`。
- `AI/`
  - AI Controller 和 AI 相关 C++ 入口。
- `Character/`
  - 角色继承体系。
  - `AuraCharacterBase`: 玩家和敌人的公共基类。
  - `AuraCharacter`: 玩家角色。
  - `AuraEnemy`: 敌人角色。
- `Game/`
  - GameMode 等游戏框架入口。
- `Input/`
  - Enhanced Input 绑定和输入配置 DataAsset 类型。
- `Interaction/`
  - C++ 接口，例如战斗接口、敌人接口。
- `Player/`
  - `AuraPlayerController`、`AuraPlayerState`。
- `UI/`
  - `HUD/`: HUD 入口。
  - `Widget/`: UMG Widget C++ 基类和组件。
  - `WidgetController/`: UI 与 GAS 数据绑定的控制器。

### 根级 C++ 文件

- `AuraGameplayTags.h/.cpp`: 原生 Gameplay Tags 的集中注册和访问。
- `AuraAbilityTypes.h/.cpp`: 自定义 GAS 类型，例如扩展的 effect context。
- `AuraAssetManager.h/.cpp`: 项目 Asset Manager 入口。
- `Aura.h/.cpp`: 模块基本入口。

## 蓝图和资产布局

蓝图主要在 `Content/Blueprints/`，原始美术、动画、声音和特效资产主要在 `Content/Assets/`。

### `Content/Blueprints`

- `AbilitySystem/`
  - 技能、Gameplay Effect、Gameplay Cue、Gameplay Tag 相关蓝图。
  - `Aura/`: 玩家 Aura 相关技能和默认属性。
  - `Enemy/`: 敌人技能、默认属性、Gameplay Cue。
- `AI/`
  - 行为树、黑板、EQS、BT Service、BT Task。
- `AnimNotify/`
  - 动画通知蓝图。
- `BuildingActor/`
  - 场景建筑或可交互建筑 Actor 蓝图。
- `Character/`
  - 玩家和敌人角色蓝图。
  - 常见子目录：`Aura/`、`Demon/`、`Ghoul/`、`Goblin_Slingshot/`、`Goblin_Spear/`、`Shaman/`。
- `EffectActor/`
  - 区域效果、晶体、药水、测试用效果 Actor。
- `Game/`
  - GameMode 等游戏框架蓝图。
- `Input/`
  - Input Action 和 Input Mapping 相关资产。
- `Player/`
  - 玩家控制器或玩家相关蓝图。
- `UI/`
  - HUD、Overlay、属性菜单、浮动伤害文本、按钮、进度条、WidgetController 数据资产。

### `Content/Assets`

- `Characters/`: 玩家角色模型、动画、材质、武器。
- `Enemies/`: 敌人模型、动画、材质和武器资源。
- `Dungeon/`: 地牢环境模型和材质。
- `Effects/`: Niagara、材质、投射物、法术、命中特效。
- `MagicCircles/`: 魔法阵资源。
- `Materials/`: 通用材质。
- `Pickups/`: 药水、晶体等拾取物资产。
- `Sounds/`: 技能、敌人、脚步等声音资源。
- `Fonts/`: UI 字体。
- `UI/`: UI 美术资源。

### `Content/Maps`

- 地图资产目录。当前项目中可见 `Dungeon.umap` 曾被修改，做地图相关改动前应先确认编辑器内资产状态。

## 配置和数据

- `Config/DefaultGameplayTags.ini`: Gameplay Tag 配置。C++ 原生 tag 主要看 `AuraGameplayTags`。
- `Config/DefaultInput.ini`: 输入相关配置。
- `Config/DefaultGame.ini`: 游戏配置，也可能包含 GAS globals 配置。
- `Config/DefaultEngine.ini`: 引擎和项目级配置。
- `Data/CT_Damage.json`: 伤害曲线表导出数据。
- `Data/CT_PrimaryAttributes_Elementalist.json`: Elementalist 主属性曲线导出数据。
- `Data/CT_PrimaryAttributes_Ranger.json`: Ranger 主属性曲线导出数据。
- `Data/CT_PrimaryAttributes_Worrior.json`: Warrior 主属性曲线导出数据，文件名保留了当前拼写。

## 学习和参考资料

- `CLAUDE.md`: 当前项目架构、GAS 管线、常见开发流程的详细说明。
- `AGENTS.md`: 面向 AI agent 的仓库规则、构建命令、测试建议和提交风格。
- `学习指导/`: 学习笔记、复习材料、课程信息和课程仓库参考副本。
- `Gameplay架构/`: UE Gameplay 框架相关文章笔记。

注意：`学习指导/课程信息/课程github仓库/` 是课程参考仓库副本，不是当前项目主源码。需要对比课程代码时可读，修改当前项目时优先操作仓库根目录下的 `Source/`、`Content/`、`Config/`。

## 常见任务定位

- 改属性：先看 `Source/Aura/Public/AbilitySystem/AuraAttributeSet.h` 和 `Source/Aura/Private/AbilitySystem/AuraAttributeSet.cpp`，再看 `AuraGameplayTags`、属性 UI DataAsset、默认属性 Gameplay Effect 蓝图。
- 改伤害公式：看 `Source/Aura/Private/AbilitySystem/ExecCalc/ExecCalc_Damage.cpp`，再检查伤害 Gameplay Effect 蓝图和曲线表。
- 改技能输入：看 `Source/Aura/Private/Player/AuraPlayerController.cpp`、`Source/Aura/Private/Input/`，再检查 `Content/Blueprints/Input/` 与 `DA_InputConfig`。
- 改玩家技能：看 `Source/Aura/Public/AbilitySystem/Abilities/` 与 `Private/AbilitySystem/Abilities/`，再检查 `Content/Blueprints/AbilitySystem/Aura/GameplayAbility/`。
- 改敌人技能或 AI：看 `Source/Aura/Private/Character/AuraEnemy.cpp`、`Source/Aura/Private/AI/`，再检查 `Content/Blueprints/AI/` 和 `Content/Blueprints/AbilitySystem/Enemy/`。
- 改 UI 数据流：看 `Source/Aura/Private/UI/WidgetController/`、`Source/Aura/Private/UI/HUD/AuraHUD.cpp`，再检查 `Content/Blueprints/UI/`。
- 改角色战斗插槽或攻击蒙太奇：看 `Source/Aura/Public/AbilitySystem/Data/CombatSocketInfo.h`、`Source/Aura/Private/Character/AuraCharacterBase.cpp`，再检查角色蓝图和对应 DataAsset。
- 改投射物：看 `Source/Aura/Private/Actor/AuraProjectile.cpp`、`Source/Aura/Private/AbilitySystem/Abilities/AuraProjectileSpell.cpp`，再检查投射物蓝图和 Niagara 资产。

## 修改前检查建议

- C++ 与 Blueprint 混合实现较多。改 gameplay C++ 前，尽量先查对应蓝图资产路径。
- 不要回滚用户已有改动。当前项目可能同时存在 `.uasset`、地图和 C++ 的未提交修改。
- `.uasset` 和 `.umap` 无法通过普通文本 diff 理解，修改前后应在 Unreal Editor 中验证引用和行为。
- 新增 C++ 类后通常需要重新生成项目文件或构建 `AuraEditor`。

## 验证入口

常用 C++ 构建命令：

```powershell
& "E:\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" AuraEditor Win64 Development -Project="E:\Epic Games\project\Aura\Aura.uproject" -WaitMutex
```

打开编辑器：

```powershell
& "E:\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe" "E:\Epic Games\project\Aura\Aura.uproject"
```

