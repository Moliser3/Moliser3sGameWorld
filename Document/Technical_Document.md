# 技术文档 — Moliser3sGameClient

> 引擎：Unreal Engine 5.7  
> 语言：C++  
> 最后更新：2026/06/22

---

## 一、项目概览

### 1.1 目录结构

```
Moliser3sGameClient/Source/
├── BaseCharacter.h/.cpp                     # 基础角色类
├── PlayerCharacter.h/.cpp                   # 玩家角色（状态监听者）
├── EnemyCharacter.h/.cpp                    # 敌人角色
├── WorldPlayerController.h/.cpp            # 玩家控制器（状态拥有者 + 输入分发）
├── WorldGameMode.h/.cpp                     # 游戏模式
├── GamePlayerState.h                        # 双轴状态枚举 + 事件委托
├── DebugHelper.h                            # Debug 宏工具
├── Component/
│   ├── Attribute/AttributeComponent.h/.cpp  # 属性组件
│   ├── Damage/DamageCalculatorComponent.h/.cpp # 伤害计算
│   ├── Equipment/EquipmentComponent.h/.cpp  # 装备管理（14槽位+五行加成）
│   ├── Facing/FacingComponent.h/.cpp        # 朝向控制组件
│   ├── Input/ClickDetectionComponent.h/.cpp # 点击检测组件
│   ├── Inventory/
│   │   ├── InventoryComponent.h/.cpp        # 背包组件（30格+堆叠+事件）
│   │   └── QuickSlotComponent.h/.cpp        # 快捷栏组件（8格+数字键）
│   ├── Camera/CameraControllerComponent.h/.cpp # 摄像机控制
│   └── Skill/SkillSystemComponent.h/.cpp     # 技能系统组件
├── Skill/
│   ├── SkillTypes.h                         # 类型定义 (ESkillCategory / ESkillType / FSkillStage)
│   ├── SkillBase.h/.cpp                     # 技能基类 (多阶段)
│   ├── DamageSkillBase.h/.cpp               # 伤害技能中间类
│   ├── MeleeSlashSkill.h/.cpp               # 扇形斩击技能
│   └── JumpSkill.h/.cpp                     # 跳跃技能 (抛物线位移)
├── Data/
│   ├── CharacterData.h                      # 角色核心数据（五行→五维→派生）
│   ├── DataDefinitions.h                    # 公共枚举（EWuXing/ESkillWuXing）
│   ├── EquipmentData.h                      # 装备/武器/稀有度枚举
│   ├── ItemBase.h                           # 物品基类（Use+WorldMesh+Icon缓存）
│   ├── EquipItem.h/.cpp                     # 可装备物品（五行加成）
│   └── ConsumableItem.h/.cpp                # 消耗品基类（回血/回蓝/增益）
└── WorldActors/
    └── WorldItemActor.h/.cpp                # 地面物品 Actor（Mesh+拾取）
```

### 1.2 编译方式

```bash
Build.bat Moliser3sGameClientEditor Win64 Development -Project="项目路径\Moliser3sGameClient.uproject"
```

---

## 二、类体系

### 2.1 角色体系

| 类名 | 父类 | 作用 | 持有组件 |
|------|------|------|---------|
| `ABaseCharacter` | `ACharacter` | 基础角色 | `UAttributeComponent`, `UDamageCalculatorComponent`, `UEquipmentComponent`, `UInventoryComponent` |
| `APlayerCharacter` | `ABaseCharacter` | 玩家角色（状态监听者） | `UFacingComponent`, `USkillSystemComponent`, `UQuickSlotComponent`, `UCameraComponent` |
| `AEnemyCharacter` | `ABaseCharacter` | 敌人角色 | 继承自 ABaseCharacter 的全部组件 |

**ABaseCharacter 关键成员：**
- `MoveToLocation(FVector)` — NavMesh 寻路移动
- `StopMovement()` — 停止移动
- `GetSpeed()` — 当前移动速度
- `WalkSpeed=300`, `RunSpeed=600`
- `GetAttributeComponent()` — 属性组件
- `GetDamageCalculator()` — 伤害计算组件
- `GetEquipmentComponent()` — 装备组件
- `GetInventory()` — 背包组件

### 2.2 控制器

| 类名 | 父类 | 作用 |
|------|------|------|
| `AWorldPlayerController` | `APlayerController` | 状态拥有者 + 输入分发 |

**状态管理：**
- `ECombatState` 战斗感知轴（Default / BattlePerception）
- `EActionState` 行为轴（Idle / Walking / Running / Skill）
- 通过事件广播驱动 PlayerCharacter 响应

**输入映射：**
| 操作 | 回调 | 行为 |
|------|------|------|
| 左键 | `OnLeftMouseClick()` | 点地面移动 / 点敌人锁定+攻击 |
| 右键 | `OnRightMouseClick()` | 释放右键技能组 |
| Shift | 内部 `IsInputKeyDown` | 行走↔奔跑切换 |
| Alt | `OnAltPressed/Released()` | 防御开关 |

### 2.3 组件体系

| 组件 | 挂载位置 | 作用 |
|------|---------|------|
| `UAttributeComponent` | 角色 | 血量/法力/攻击/防御属性 |
| `UDamageCalculatorComponent` | 角色 | 最终伤害计算（含暴击、防御减免） |
| `UEquipmentComponent` | 角色 | 装备管理（14槽位+双手武器锁定+五行加成应用） |
| `UInventoryComponent` | 角色 | 背包（30格+堆叠+拾取/丢弃/使用+事件广播） |
| `UQuickSlotComponent` | 玩家 | 快捷栏（8格+数字键触发） |
| `UFacingComponent` | 玩家 | Walking/Aiming 两种朝向模式 |
| `UClickDetectionComponent` | Controller | 屏幕鼠标射线检测 |
| `USkillSystemComponent` | 玩家 | 双技能组管理 + 四阶段状态机 |
| `UCameraControllerComponent` | Controller | 双轴独立弹性相机跟随 |

---

## 三、双轴状态系统

### 3.1 设计理念

行为由两根正交轴组合定义，状态变化通过事件广播驱动响应方，消除分散的条件判断。

### 3.2 状态枚举

```cpp
UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Default          UMETA(DisplayName = "默认"),
    BattlePerception UMETA(DisplayName = "战斗感知")
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
    Idle    UMETA(DisplayName = "闲置"),
    Walking UMETA(DisplayName = "行走"),
    Running UMETA(DisplayName = "奔跑"),
    Skill   UMETA(DisplayName = "释放技能")
};
```

### 3.3 组合行为矩阵

| Combat | Action | 面朝 | 速度 | 说明 |
|--------|--------|------|------|------|
| Default | Idle | — | 0 | 初始/停止 |
| Default | Walking | 移动方向 | 300 | 普通行走 |
| Default | Running | 移动方向 | 600 | Shift 奔跑 |
| Default | Skill | 点击方向 | 0 | 释放技能 |
| Battle | Idle | 目标 | 0 | 锁定敌人待命 |
| Battle | Walking | **目标** | 300 | 面朝敌人行走 |
| Battle | Running | **移动方向** | 600 | 奔跑，到达后恢复面朝目标 |
| Battle | Skill | 目标/点击方向 | 0 | 释放技能 |

### 3.4 状态转移

| 编号 | 转移 | 触发条件 | 所在位置 |
|------|------|---------|---------|
| 1 | Combat: Default → Battle | 左键/右键点击敌人 | `OnLeft/RightMouseClick` |
| 2 | Combat: Battle → Default | AimTarget 丢失 或 距离 > BattlePerceptionRange | Tick Section 1 |
| 3 | Action: Idle → Walking | 左键点击地面 (无 Shift) | `OnLeftMouseClick` |
| 4 | Action: Idle → Running | Shift + 左键点击地面 | `OnLeftMouseClick` |
| 5 | Action: Idle → Skill | 右键点击/自动攻击触发 | `OnRightMouseClick` / Tick Section 4 |
| 6 | Action: Running → Idle | VelSq < 100 + 延时 > 0.3s | Tick Section 3 |
| 7 | Action: Skill → Idle | 技能执行完毕 (Phase = Idle) | Tick Section 2 |

### 3.5 事件回调链

```
WorldPlayerController (状态拥有者)
  ├── OnCombatStateChanged(ECombatState)
  │   └── PlayerCharacter::OnCombatStateChanged()
  │       ├── Battle  → FacingComp->SetMode(Aiming)
  │       └── Default → FacingComp->ClearAimTarget()
  └── OnActionStateChanged(EActionState)
      └── PlayerCharacter::OnActionStateChanged()
          ├── Idle   → MaxWalkSpeed = 0
          ├── Walking→ MaxWalkSpeed = WalkSpeed (300)
          ├── Running→ MaxWalkSpeed = RunSpeed (600)
          └── Skill  → StopMovement()

Tick 中的额外逻辑 (Controller):
  ├── Section 1: 战斗感知 → 默认 (距离/目标检测)
  ├── Section 2: 技能 → Idle (检测 Phase 变化)
  ├── Section 3: 奔跑 → Idle (速度 + 延时检测)
  ├── Section 4: Idle+Battle+有目标且非Aiming → SetMode(Aiming)
  └── Section 5: bPendingAttack 自动攻击
```

### 3.6 战斗感知范围

- `APlayerCharacter::BattlePerceptionRange`（蓝图可配置，默认 1500cm）
- 超出此距离且 `AimTarget` 有效 → 自动退出战斗状态
- `FacingComponent` 不再负责距离检测（由 Controller 统一管理）

---

## 四、技能系统

### 4.1 类继承结构

```
USkillBase                        技能基类 (Stages 多阶段数组)
  ├─ UDamageSkillBase             伤害技能中间类 (ApplyDamage)
  │   └─ UMeleeSlashSkill         扇形斩击 (从 Stages[CurrentStage] 读取伤害参数)
  └─ UJumpSkill                   跳跃位移 (抛物线，参数在 Stages[0])
```

### 4.2 FSkillStage 结构

```cpp
USTRUCT(BlueprintType)
struct FSkillStage
{
    float WindupTime          // 前摇
    float RecoveryTime        // 后摇
    float CustomLinkTime      // 衔接时间
    UAnimMontage* SkillMontage // 蒙太奇
    FName MontageSlotName     // 槽位
    float BaseDamage          // 基础伤害
    float HalfAngleDeg        // 扇形半角
    float MaxZDiff            // 最大高度差
};
```

### 4.3 技能阶段推进规则

```
连续同类型(ESkillType) → StageForType++
不同类型              → StageForType = 0
越界                  → StageForType = 0

[A][A][A][B][B][A][B]
 0  1  2  0  1  0  0    ← StageForType 值
```

### 4.4 双技能组

| 组件 | 绑定的输入 | 行为 |
|------|-----------|------|
| `LeftSkillGroup` | 左键 | 距离检查 → 移动接近 → 攻击 |
| `RightSkillGroup` | 右键 | 无距离检查，直接释放 |

**独立追踪：**
- `LeftGroupIndex` / `RightGroupIndex` — 组索引
- `LeftLastSkillType` / `RightLastSkillType` — 上一技能类型
- `LeftStageForType` / `RightStageForType` — 阶段进度

### 4.5 四阶段状态机

```
ActivateLeft/Right()
      │
      ▼
  ┌────────┐  点击   ┌──────────┐
  │  Idle  │───────→│  Windup   │ → OnWindupUpdate → 前摇到→ Recovery
  └────────┘        └──────────┘
       ▲                  │
       │                  ▼
       │           ┌──────────┐
       │           │ Recovery  │── 点击 → OnInterrupt → 下一技能
       │           └────┬─────┘
       │                │ 后摇到
       │                ▼
       │           ┌──────────┐
       │           │LinkWindow│── 点击 → 下一技能
       │           └────┬─────┘
       │                │ 超时
       │                ▼
       └────── 重置阶段追踪 ──→ Idle
```

### 4.6 技能打断与衔接

| 阶段 | 可打断 | 行为 |
|------|--------|------|
| Windup | ❌ | 前摇不可打断 |
| Recovery | ✅ | 点击 → OnInterrupt → 下一技能 |
| LinkWindow | ✅ | 点击 → 下一技能；超时 → 索引归零 |

**衔接超时重置：**
- `LeftGroupIndex` / `RightGroupIndex` 归零
- `LeftStageForType` / `RightStageForType` 归零
- `LeftLastSkillType` / `RightLastSkillType` 重置为 None

---

## 五、输入系统

### 5.1 左键

```
IA_LeftClick Triggered
  ↓
OnLeftMouseClick()
  ↓
[点击敌人]
  ├─ FacingComp->SetAimTarget(敌人)
  ├─ SetCombatState(Battle)
  ├─ 距离检查
  │   ├─ 超距 → SetActionState(Walking) + bPendingAttack
  │   └─ 在范围 → SetActionState(Skill) + ActivateLeft()
  │
[点击地面]
  ├─ Shift → FacingComp->SetMode(Walking) + SetActionState(Running)
  │          + RunStartTime
  ├─ Battle + 无Shift → FacingComp->SetMode(Aiming) + SetActionState(Walking)
  └─ Default + 无Shift → SetActionState(Walking)
  ↓
MoveToLocation(点击位置)
```

### 5.2 右键

```
IA_RightClick Triggered
  ↓
OnRightMouseClick()
  ↓
[点击敌人]
  ├─ FacingComp->SetAimTarget(敌人)
  └─ SetCombatState(Battle)
  ↓
SetActionState(Skill) + ActivateRight()
```

### 5.3 Shift

`IsInputKeyDown(EKeys::LeftShift)` 在 `OnLeftMouseClick` 中直接判断，无需蓝图输入绑定。

### 5.4 Alt

```cpp
OnAltPressed()  → bDefending = true
OnAltReleased() → bDefending = false
```

蓝图输入事件绑定 Alt 键。

---

## 六、角色移动与朝向

### 6.1 速度控制

| Action | 速度来源 | 默认值 |
|--------|---------|-------|
| Idle | 0 | 0 |
| Walking | `GetWalkSpeed()` | 300 |
| Running | `GetRunSpeed()` | 600 |
| Skill | 0 (StopMovement) | 0 |

速度由 `PlayerCharacter::OnActionStateChanged` 统一设置，Controller 不再直接干预。

### 6.2 FacingComponent 朝向模式

| 模式 | 说明 | `bOrientRotationToMovement` |
|------|------|----------------------------|
| Walking | 面朝移动方向 | true |
| Aiming | 面朝锁定目标 | false |

**公共接口：**
- `SetAimTarget(Actor)` — 设置目标 + 进入 Aiming 模式
- `SetMode(EFacingMode)` — 仅切换朝向模式，保留目标不变
- `ClearAimTarget()` — 清除目标 + 回到 Walking 模式
- `GetCurrentFacingMode()` — 获取当前模式
- `GetAimTarget()` — 获取当前注视目标

### 6.3 状态组合 × 面朝规则

| Combat | Action | 面朝控制 | 实现 |
|--------|--------|---------|------|
| Default | Walking | 移动方向 | `bOrientRotationToMovement = true` |
| Default | Running | 移动方向 | `bOrientRotationToMovement = true` |
| Battle | Walking | 目标 | `FacingComp->SetMode(Aiming)` |
| Battle | Running | 移动方向 | `FacingComp->SetMode(Walking)` → 到达后 Idle → Tick 恢复 Aiming |

---

## 七、数据流

### 7.1 左键攻击流程

```
左键点击敌人
  ├─ SetAimTarget + SetCombatState(Battle)
  ├─ GetMaxSkillRange() 距离检查
  │   ├─ 超距 → SetActionState(Walking) + MoveToLocation + bPendingAttack
  │   │         Tick → 到达范围 → SetActionState(Skill) + ActivateLeft()
  │   └─ 在范围 → SetActionState(Skill) + ActivateLeft()
  │
  ▼
ActivateLeft()
  └─ ExecuteSkillFromGroup(LeftSkillGroup, LeftGroupIndex, ...)
      ├─ 取 LeftGroup[LeftGroupIndex]
      ├─ 阶段判定 (连续同类型推进)
      ├─ SetCurrentStage(StageForType)
      └─ Skill->Execute(Owner) → Windup 开始
```

### 7.2 右键释放流程

```
右键点击
  ├─ 点击敌人 → SetAimTarget + SetCombatState(Battle)
  └─ 记录 LastClickTarget
  │
  ▼
SetActionState(Skill) + ActivateRight()
  └─ ExecuteSkillFromGroup(RightSkillGroup, RightGroupIndex, ...)
      ├─ 无距离检查（直接释放）
      ├─ 阶段判定
      └─ Skill->Execute(Owner) → Windup 开始
```

### 7.3 奔跑恢复注视流程

```
Shift + 左键地面 (Battle)
  ├─ FacingComp->SetMode(Walking)  ← 目标保留，仅切朝向
  ├─ SetActionState(Running)
  ├─ RunStartTime = Now
  └─ MoveToLocation

Tick (每帧):
  ├─ Section 3: 速度 < 100 + 延时 > 0.3s → SetActionState(Idle)
  └─ Section 4: Battle + Idle + 有目标 + 非Aiming → SetMode(Aiming) ✅
```

---

## 八、蓝图配置指引

### 8.1 双技能组配置

在角色蓝图：
1. 选中 `SkillSystemComponent`
2. 在 `LeftSkill > LeftSkillGroup` 中添加左键技能实例
3. 在 `RightSkill > RightSkillGroup` 中添加右键技能实例

### 8.2 技能阶段配置

每个技能实例需设置：

| 字段 | DisplayName | 说明 |
|------|------------|------|
| `SkillName` | 技能名称 | 标识 |
| `SkillType` | 技能类型 | 阶段推进匹配依据 |
| `MaxSkillRange` | 最大释放技能距离 | 左键攻击距离检查 |
| `SkillCategory` | 技能分类 | Attack/Movement/Utility/Hybrid |
| `Stages[0..N]` | 技能阶段列表 | 每个阶段独立配置 |

**阶段参数：**

| 字段 | DisplayName | 说明 |
|------|------------|------|
| `WindupTime` | 前摇 | 阶段起手时间 |
| `RecoveryTime` | 后摇 | 阶段收尾时间 |
| `CustomLinkTime` | 衔接时间 | 后摇后等待时间 |
| `SkillMontage` | 蒙太奇 | 阶段动画 |
| `BaseDamage` | 基础伤害 | 该阶段伤害值 |
| `HalfAngleDeg` | 扇形半角 | 扇形检测角度 |
| `MaxZDiff` | 最大高度差 | 高度容差 |

### 8.3 战斗感知范围

`APlayerCharacter` > `Custom` > `BattlePerceptionRange`（默认 1500）

---

## 九、开发日志

| 日期 | 修改内容 | 涉及文件 |
|------|---------|---------|
| 06/22 | **[重构] 技能范围改为阶段级**：`MaxSkillRange` 移至 `FSkillStage.SkillRange` | SkillTypes.h, SkillBase.h/.cpp, MeleeSlashSkill.cpp, JumpSkill.cpp, SkillSystemComponent.cpp |
| 06/22 | **[新增] 背包系统**：InventoryComponent(30格) + QuickSlotComponent(8格) | Component/Inventory/* |
| 06/22 | **[新增] UConsumableItem 消耗品基类** | Data/ConsumableItem.h/.cpp |
| 06/22 | **[新增] AWorldItemActor 地面物品**：Mesh+Widget+拾取 | WorldActors/WorldItemActor.h/.cpp |
| 06/22 | **[扩展] UItemBase**：添加 Use() + WorldMesh + GetIcon/GetWorldMesh | Data/ItemBase.h |
| 06/22 | **[重构] 防御百分比化**：固定减法 → 防/(防+100) | DamageCalculatorComponent.cpp |
| 06/22 | **[清理] 装备系统**：删除消耗品槽位设计 | EquipmentComponent.h/.cpp, EquipmentData.h |
| 06/22 | **[新增] Debug 装备/背包测试**：13槽位+背包数组直接配 | BaseCharacter.h/.cpp |
| 06/22 | **[优化] 伤害日志中文化**：全中文+装备加成显示 | MeleeSlashSkill.cpp |
| 06/18 | **[重构] 正交状态机**：ECombatState + EActionState 双轴驱动 | GamePlayerState.h, WorldPlayerController.h/.cpp, PlayerCharacter.h/.cpp |
| 06/18 | **[新增] FacingComponent::SetMode()**：仅切换朝向不丢目标 | FacingComponent.h/.cpp |
| 06/18 | **[清理] 移除** PendingEnemyTarget / PendingAimTarget / EPlayerState 等旧变量 | WorldPlayerController.h/.cpp, PlayerCharacter.h/.cpp |
| 06/15 | **[重构] 多阶段技能系统 + 双技能组 + 左键恢复** | 全部 Skill 文件 + Controller |
| 06/15 | **[新增] 玩家行为状态系统**：EPlayerState(Default/Battle) | WorldPlayerController, PlayerCharacter |
| 06/14 | **[重构] 技能系统三段式生命周期** | SkillBase, SkillSystemComponent |
| 06/13 | **[修复] 跳跃后摇期无法打断** | JumpSkill, SkillSystemComponent |
