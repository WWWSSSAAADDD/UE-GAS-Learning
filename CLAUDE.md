# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在此代码库中工作时提供指导。

## 项目概述

Aura 是一个使用 Unreal Engine 5.3 和 Gameplay Ability System (GAS) 构建的动作 RPG 游戏。项目实现了支持多人联机的战斗系统，包含属性、技能、伤害计算、AI 敌人和 UI 组件。

**引擎**: Unreal Engine 5.3  
**语言**: C++ 结合 Blueprint  
**核心系统**: Gameplay Ability System, Enhanced Input, AI (行为树, EQS), Niagara VFX

## 构建命令

项目使用位于 `E:\Epic Games\UE_5.3` 的 Unreal Build Tool (UBT)。

**构建游戏 (Development)**:
```bash
cd "E:\Epic Games\UE_5.3"
Engine/Build/BatchFiles/Build.bat Aura Win64 Development "E:\Epic Games\project\Aura\Aura.uproject" -waitmutex
```

**构建编辑器 (Development)**:
```bash
cd "E:\Epic Games\UE_5.3"
Engine/Build/BatchFiles/Build.bat AuraEditor Win64 Development "E:\Epic Games\project\Aura\Aura.uproject" -waitmutex
```

**清理构建**:
```bash
cd "E:\Epic Games\UE_5.3"
Engine/Build/BatchFiles/Clean.bat AuraEditor Win64 Development "E:\Epic Games\project\Aura\Aura.uproject" -waitmutex
```

**生成项目文件** (添加新 C++ 类后):
```bash
cd "E:\Epic Games\UE_5.3"
Engine/Build/BatchFiles/RunUBT.bat -projectfiles -vscode -project="E:\Epic Games\project\Aura\Aura.uproject" -game -engine -dotnet
```

其他可用的构建配置: Debug, DebugGame, Test, Shipping。

## 架构

### Gameplay Ability System (GAS) 实现

项目大量使用 GAS 实现所有游戏机制。核心组件：

**技能系统组件**: `UAuraAbilitySystemComponent`
- 通过输入标签管理技能激活
- 广播 gameplay effect 资产标签用于 UI 更新
- 处理技能输入（按下/释放/持续按住）

**属性集**: `UAuraAttributeSet`
- **主属性**: Strength（力量）, Intelligence（智力）, Resilience（韧性）, Vigor（活力）
- **次级属性**: Armor（护甲）, ArmorPenetration（护甲穿透）, BlockChance（格挡几率）, CriticalHitChance（暴击几率）, CriticalHitDamage（暴击伤害）, CriticalHitResistance（暴击抗性）, HealthRegeneration（生命回复）, ManaRegeneration（法力回复）, MaxHealth（最大生命）, MaxMana（最大法力）
- **抗性属性**: FireResistance（火焰抗性）, ArcaneResistance（奥术抗性）, LightingResistance（闪电抗性）, PhysicalResistance（物理抗性）
- **生命属性**: Health（生命值）, Mana（法力值）
- **元属性**: InComingDamage（仅服务器，用于伤害计算）
- 所有属性都通过 OnRep 函数进行网络复制
- 使用 `TagsToAttributes` 映射将属性绑定到 gameplay tags（用于属性菜单 UI）

**自定义 Gameplay Effect Context**: `FAuraGameplayEffectContext`
- 扩展 `FGameplayEffectContext`，添加自定义字段：`bIsBlocked`（是否格挡）, `bIsCritical`（是否暴击）
- 实现自定义序列化以支持网络复制
- 用于在伤害管线中传递战斗结果信息（格挡、暴击）

**伤害计算**: `UExecCalc_Damage`
- 用于复杂伤害公式的执行计算
- 捕获来源（攻击者）和目标（防御者）的属性
- 考虑因素：伤害系数（来自曲线表）、护甲、护甲穿透、格挡几率、暴击机制、伤害类型和抗性值
- 使用 `SetByCaller` 设置伤害类型标签（Fire, Arcane, Lighting, Physical）
- 在自定义 effect context 中设置 `bIsBlocked` 和 `bIsCritical`

**角色职业系统**: `UCharacterClassInfo` (DataAsset)
- 定义三种敌人职业：Elementalist（元素使）, Warrior（战士）, Ranger（游侠）
- 每个职业有独特的主属性和初始技能
- 所有职业共享通用的次级/生命属性和技能
- 包含 `DamageCoefficients` 曲线表用于基于等级的伤害缩放

### 角色继承体系

**基类**: `AAuraCharacterBase`
- 实现 `IAbilitySystemInterface` 和 `ICombatInterface` 接口
- 管理 ASC 和 AttributeSet 的初始化
- 通过 GameplayEffects 处理默认属性初始化
- 为角色授予初始技能
- 实现死亡系统，包含溶解材质效果和布娃娃物理
- 存储战斗插槽（WeaponTip, LeftHand, RightHand）和攻击蒙太奇

**玩家角色**: `AAuraCharacter`
- 在客户端和服务器上初始化 ASC/AttributeSet（支持多人联机）
- 通过 `UAuraInputComponent` 集成增强输入系统

**敌人角色**: `AAuraEnemy`
- 使用 `ECharacterClass` 枚举确定属性/技能
- 从 `UCharacterClassInfo` DataAsset 初始化
- 实现 `IEnemyInterface` 用于高亮显示和战斗交互
- 拥有血条组件，闲置后自动隐藏

### AI 系统

**AI 控制器**: `AAuraAIController`
- 使用行为树进行决策
- 黑板键值：TargetToFollow（跟随目标）, RangedAttackPosition（远程攻击位置）

**行为树**: `BT_EnemyBehaviorTree`
- 任务：`BTT_Attack`（触发近战/远程攻击）, `BTT_ChangePosition`（使用 EQS 寻找远程攻击位置）
- 服务：`BTS_FindNearestActor`（更新目标追踪）

**EQS (环境查询系统)**: `EQ_FindRangedAttackPosition`
- 在玩家周围寻找远程攻击的最佳位置

### 技能系统

**基础技能**: `UAuraGameplayAbility`
- 所有 gameplay abilities 都继承自此类

**伤害技能**: `UAuraDamageGameplayAbility`
- 造成伤害的技能的基类
- 处理伤害类型设置和应用

**投射物法术**: `UAuraProjectileSpell`
- 生成带追踪行为的投射物（`AAuraProjectile`）
- 使用 `UTargetDataUnderMouse` ability task 进行目标定位

**近战攻击**: `UAuraMeleeAttack`
- 处理武器碰撞的近距离战斗

**投射物 Actor**: `AAuraProjectile`
- 集成 Niagara VFX
- 球形碰撞用于命中检测
- 撞击时通过 GameplayEffect 应用伤害

### UI 系统 (UMG + GAS 集成)

**Widget Controllers**: 使用 MVC 模式将 UI 逻辑与组件分离
- `UAuraWidgetController`（基类）
- `UOverlayWidgetController`：管理 HUD 覆盖层（生命/法力条、消息组件）
- `UAttributeMenuWidgetController`：管理属性菜单显示

**HUD**: `AAuraHUD`
- 创建和管理 widget controllers
- UI 初始化的入口点

**伤害文本**: `UDamageTextComponent`
- 由 `UAuraAttributeSet::ShowFloatingText()` 动态生成
- 显示浮动伤害数字，带格挡/暴击指示器
- Client RPC 确保在多人游戏中正确显示

### 输入系统

**增强输入**: 使用 Input Actions 和 Input Mapping Context
- `UAuraInputConfig`：DataAsset，将 gameplay tags 映射到输入动作
- `UAuraInputComponent`：自定义组件，处理技能输入绑定
- 输入标签：LMB, RMB, 1-4, Shift

**玩家控制器**: `AAuraPlayerController`
- 处理光标追踪用于目标定位
- 管理输入路由到技能
- 在目标上生成伤害文本组件

### Gameplay Tags

**原生标签**: 在 `FAuraGameplayTags` 单例中定义
- 所有主属性/次级属性/生命属性/抗性属性的标签
- 用于技能激活的输入标签
- 伤害类型标签（Fire, Arcane, Lighting, Physical）映射到抗性标签
- 技能标签（Abilities_Attack）
- 效果标签（Effects_HitReact）
- 蒙太奇标签（Montage_Attack_Weapon, Montage_Attack_LeftHand, Montage_Attack_RightHand）

### 多人联机注意事项

- ASC 初始化在玩家（客户端 + 服务器）和 AI（仅服务器）之间有所不同
- 属性使用 OnRep 函数进行网络复制
- 自定义 GameplayEffectContext 实现 NetSerialize 用于复制
- 伤害文本使用 Client RPC 在所有客户端显示
- 死亡处理使用 NetMulticast RPC 进行布娃娃同步
- 元属性（InComingDamage）仅在服务器端，不进行复制

### 数据资产和表格

**DataAssets**:
- `DA_AttributeInfo`：用于 UI 显示的属性元数据
- `DA_InputConfig`：输入动作到 gameplay tag 的映射
- `DA_DefaultAttributes`：角色职业默认属性

**DataTables**:
- `DT_MessageWidgetData`：将 gameplay tags 映射到 UI 消息组件
- `CT_DamageCoefficient`：按等级缩放的伤害系数
- `CT_PrimaryAttributes_*`：按角色职业和等级的主属性值

### 动画系统

- 动画蓝图：`ABP_Aura`, `ABP_Enemy`, `ABP_Ghoul`, `ABP_Goblin_Spear`, `ABP_Goblin_Slingshot`
- 受击蒙太奇由 `GA_HitReact` 技能触发
- 攻击蒙太奇带有 `AN_MontageEvent` 通知，用于伤害应用时机
- Motion Warping 用于角色朝向目标旋转

## 代码规范

- 对 UPROPERTY 指针使用 `TObjectPtr<>`（UE5 标准）
- 所有成员指针必须有 `UPROPERTY()` 宏以防止垃圾回收
- 复制属性使用 `ReplicatedUsing` 配合 OnRep 函数
- 接口实现使用 `_Implementation` 后缀
- 原生 gameplay tags 在 `FAuraGameplayTags::InitializeNativeGameplayTags()` 中初始化
- 属性访问器使用 `ATTRIBUTE_ACCESSORS` 宏
- 代码库中使用中文注释进行文档说明

## 常见工作流程

**添加新属性**:
1. 在 `UAuraAttributeSet` 中添加 `FGameplayAttributeData` 属性，使用 `ATTRIBUTE_ACCESSORS` 宏
2. 在 `GetLifetimeReplicatedProps()` 中添加，使用 `DOREPLIFETIME_CONDITION_NOTIFY` 宏
3. 创建 OnRep 函数
4. 在 `FAuraGameplayTags` 中添加 gameplay tag
5. 在 AttributeSet 构造函数中添加到 `TagsToAttributes` 映射
6. 如果是次级属性，更新 MMC（Modifier Magnitude Calculation）类

**添加新技能**:
1. 创建继承自 `UAuraGameplayAbility` 或 `UAuraDamageGameplayAbility` 的 C++ 类
2. 在 `Content/Blueprints/AbilitySystem/` 中创建蓝图子类
3. 添加到角色的 `StartupAbilities` 数组或 `CharacterClassInfo` DataAsset
4. 创建用于技能激活的 gameplay tag
5. 如果是玩家激活的技能，在 `DA_InputConfig` 中将输入动作映射到标签

**添加新敌人**:
1. 创建继承自 `BP_AuraEnemyBase` 的蓝图
2. 设置 `CharacterClass` 枚举（Elementalist/Warrior/Ranger）
3. 配置骨骼网格和动画
4. 设置战斗插槽和攻击蒙太奇
5. 属性/技能会从 `DA_DefaultAttributes` 自动初始化

**修改伤害计算**:
- 编辑 `ExecCalc_Damage.cpp` 中的 `UExecCalc_Damage::Execute_Implementation()`
- 伤害公式考虑系数、护甲、穿透、格挡、暴击和抗性
- 使用曲线表进行基于等级的缩放
