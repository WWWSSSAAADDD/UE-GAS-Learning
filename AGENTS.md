# Repository Guidelines

## 项目信息
- 本项目是跟随udemy课程学习的学习项目
- udemy课程地址：https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/
- 课程GitHub项目：https://github.com/DruidMech/GameplayAbilitySystem_Aura

## Project Structure & Module Organization

This is an Unreal Engine 5.3 project. The runtime C++ module is `Aura`.

- `Source/Aura/Public`: public headers grouped by feature, such as `AbilitySystem`, `Actor`, `Character`, `Player`, `UI`, and `Interaction`.
- `Source/Aura/Private`: matching implementation files. Keep new `.cpp` files under the same feature folder as their public API.
- `Content`: Unreal assets, Blueprints, maps, animations, effects, and UI assets.
- `Config`: project, input, gameplay tag, and engine configuration.
- `Data`: exported or helper data files, including curve table JSON snapshots.
- `Plugins/VisualStudioTools`: bundled editor integration plugin; avoid editing it unless the task is plugin-specific.

Generated folders such as `Binaries`, `Intermediate`, `Saved`, and `DerivedDataCache` are not source.

## Build, Test, and Development Commands

Use the UE 5.3 toolchain associated with this project:

```powershell
& "E:\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" AuraEditor Win64 Development -Project="E:\Epic Games\project\Aura\Aura.uproject" -WaitMutex
```

Builds the editor target for local C++ validation.

```powershell
& "E:\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe" "E:\Epic Games\project\Aura\Aura.uproject"
```

Opens the project in the editor for Blueprint, asset, and gameplay checks.

## Coding Style & Naming Conventions

Follow `.clang-format`: LLVM base, 4-space tab width, tabs for indentation, Allman braces, no column limit, and preserved include ordering. Follow Unreal naming from `.editorconfig`: `A` actors, `U` UObject types, `F` structs, `E` enums, `T` templates, and `b` booleans. Prefer feature-local helpers and existing GAS patterns.

## Testing Guidelines

No dedicated test suite is currently present. For C++ changes, build `AuraEditor` and perform an editor playtest for the affected flow. For ability or attribute changes, verify server-authoritative behavior in multiplayer PIE where relevant. Name future automation tests after the feature path, for example `Aura.AbilitySystem.ProjectileDamage`.

## Commit & Pull Request Guidelines

Recent commits use concise Chinese imperative summaries describing the system changed and action taken, such as refactoring combat sockets or adding AI behavior. Keep commits focused. PRs should include a behavior summary, affected assets or Blueprints, validation steps, and screenshots or clips for visible gameplay/UI changes. Call out migration steps for renamed assets, gameplay tags, or data tables.

## Agent-Specific Instructions

Do not revert user changes or unrelated asset edits. Before modifying gameplay C++, inspect the matching Blueprint usage when possible. Keep changes scoped, and prefer explanations when the user is learning rather than applying broad fixes automatically.
