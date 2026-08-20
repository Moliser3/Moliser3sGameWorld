# UE Project Context

*Last updated: 2026-08-20*

## Engine & Project Overview
**Engine version:** UE 5.8 — Launcher build
**Project name:** Moliser3sGameWorld（客户端工程 Moliser3sGameClient）
**Description:** UE5 ARPG 游戏（动作角色扮演），右键操作驱动的点击式移动/战斗
**Project type:** 游戏
**Genre / domain:** ARPG
**Target platforms:**
- Windows（Desktop，默认 DX12 / SM6，Ray Tracing + Lumen + Virtual Shadow Maps）
- 工程含 Linux/Mac RHI 配置，当前以 Windows 为主

## Module Structure
**Primary game module:** Moliser3sGameClient

| Module | Type | Notes |
|--------|------|-------|
| Moliser3sGameClient | Runtime | 主游戏模块（全部玩法代码） |

**Key dependencies per module:**
- **Moliser3sGameClient**: PublicDeps: Core, CoreUObject, Engine, InputCore, EnhancedInput; PrivateDeps: AIModule, NavigationSystem, UMG, Slate, SlateCore, ProceduralMeshComponent

## Plugin Dependencies
**Engine plugins enabled:**
- ModelingToolsEditorMode — 编辑器建模工具（Editor only）
- EditorToolset — 编辑器工具集（Editor only）
- NiagaraToolsets — Niagara 特效工具集（Editor only）
- ProceduralMeshComponent — 运行时程序化网格（用于路径显示）
- MCPClientToolset — MCP 客户端工具集
- ModelContextProtocol — 模型上下文协议（与外部 AI/MCP 服务通信）

**Marketplace / Fab plugins:** 无

**Custom plugins:** 无（工程内无自定义 Plugins/ 目录）

## Coding Conventions
**Naming prefixes:** 标准 UE（F/U/A/E/I）
**Header style:** `#pragma once`
**Log categories in use:** 未定义自定义 Log Category（暂无 DEFINE_LOG_CATEGORY）
**Assertion style:** 未确定（工程中未见统一约定）
**Header organization:** Public/Private 分目录，子目录按功能分组（Component/ Data/ Skill/ UI/ WorldActors）
**Additional rules:**
- 代码标识符（类名/变量/方法/枚举值）一律使用英文
- 注释、蓝图 DisplayName、日志输出等面向用户/开发者的提示信息使用中文
- UObject 成员指针使用 `TObjectPtr<>`；资源引用偏好 `TSoftObjectPtr<>`（图标、网格等异步加载）
- 角色/物品/技能类大量使用 `Blueprintable` + `Instanced + EditInlineNew`，支持蓝图内直接配置
- 组件类统一 `ClassGroup=(Custom), meta=(BlueprintSpawnableComponent)`

## Subsystems in Use
**Gameplay framework:**
- GameMode: `AWorldGameMode`（AGameModeBase，默认 Pawn 为 APlayerCharacter）
- GameState: `AWorldGameState`（AGameStateBase，暂未扩展）
- PlayerController: `AWorldPlayerController`（APlayerController，Enhanced Input + 点击检测 + 战斗/行动状态）
- PlayerState: `AWorldPlayerState`（APlayerState，暂未扩展；状态枚举/委托定义在 `GamePlayerState.h`）
- HUD: `AWorldHUD`（AHUD，暂未扩展）
- Character: `APlayerCharacter`（继承 ABaseCharacter → ACharacter）；敌人 `AEnemyCharacter`（ABaseCharacter）

**Subsystems:** 未使用自定义 UGameInstanceSubsystem/UWorldSubsystem/ULocalPlayerSubsystem/UEngineSubsystem

**Custom systems:**
- 技能系统：`USkillBase` 及其子类（`USkillBase`/`UDamageSkillBase`/`UJumpSkill`/`UMeleeSlashSkill`）+ `USkillSystemComponent` + `SkillTypes.h`。三段式生命周期：前摇(Windup) → 触发(OnExecute) → 后摇(Recovery) → 衔接(Link)；技能由蓝图的 `SkillGroup` 数组配置，组内循环释放
- 物品/装备系统：`ItemBase/EquipItem/ConsumableItem/ItemFactory`，DataTable 驱动（`ItemDataTable.h`），装备槽驱动属性（AttributeComponent + EquipmentComponent）
- 属性系统：`UAttributeComponent`（生命/法力 + 变更委托）
- 伤害计算：`UDamageCalculatorComponent`
- 背包/快捷栏：`UInventoryComponent` / `UQuickSlotComponent`（拖拽 UI 支持）
- 移动/寻路：`ABaseCharacter::MoveToLocation`（NavMesh）+ `UPathDisplayComponent`（ProceduralMesh 绘制路径）+ `UFacingComponent`（朝向控制）
- 相机/输入：`UCameraControllerComponent` + `UClickDetectionComponent` + Enhanced Input
- 世界物品：`AWorldItemActor`（StaticMesh + Widget 组件）
- 状态定义（GamePlayerState.h）：`ECombatState`（Default/BattlePerception）、`EActionState`（Idle/Walking/Running/Skill）及对应动态多播委托

**GAS usage:** 未使用 Gameplay Ability System。技能为自定义实现，涉及技能/效果需求时参考上面的自定义技能系统而非 GAS。

## Build Configuration
**Build targets:** Game、Editor（无独立 Server/Client Target）
**Custom macros / build flags:** 无
**Third-party libraries:** 无
**Platform-specific notes:**
- Windows: 默认 DX12，D3D12 目标 SM6，D3D11 目标 SM5
- Linux: Vulkan SM6；Mac: Metal SM6
- 自定义碰撞通道：`FluidTrace`（ECC_GameTraceChannel1，trace 类型），并覆盖了若干默认碰撞 Profile
**Engine modifications:** 无

## Team Context
**Team size:** 单人（项目拥有者 Moliser3 先生）
**Source control:** Git
**Branching strategy:** 未特别约定
**Code review:** 无正式流程
**Documentation standards:** `Document/` 目录存有项目常识/开发计划/当前状态等技术文档；AI 助手全程使用中文交流，git commit 描述必须用中文