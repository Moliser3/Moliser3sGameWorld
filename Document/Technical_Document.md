# 技术文档 — Moliser3sGameClient

> 引擎：Unreal Engine 5.7  
> 语言：C++  
> 最后更新：2026/06/15 14:00

---

## 一、项目概览

### 1.1 项目结构

```
Moliser3sGameClient/Source/
├── BaseCharacter.h/.cpp          # 基础角色类
├── PlayerCharacter.h/.cpp        # 玩家角色（技能由蓝图 SkillGroup 配置）
├── EnemyCharacter.h/.cpp         # 敌人角色
├── WorldPlayerController.h/.cpp  # 玩家控制器（右键全功能）
├── WorldGameMode.h/.cpp          # 游戏模式
├── Component/
│   ├── Attribute/                # 属性组件
│   ├── Damage/                   # 伤害计算组件
│   ├── Facing/                   # 朝向控制组件
│   ├── Input/                    # 点击检测组件
│   ├── Camera/                   # 摄像机控制器组件
│   └── Skill/                    # 技能系统组件（状态机）
└── Skill/
    ├── SkillTypes.h              # 枚举定义（ESkillCategory）
    ├── SkillBase.h/.cpp          # 技能基类（前摇/后摇/衔接时间）
    ├── DamageSkillBase.h/.cpp    # 伤害技能中间类
    ├── MeleeSlashSkill.h/.cpp    # 扇形斩击技能
    └── JumpSkill.h/.cpp          # 跳跃技能（抛物线位移）
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
| `ABaseCharacter` | `ACharacter` | 基础角色，持有通用属性和移动接口 | `UAttributeComponent`, `UDamageCalculatorComponent` |
| `APlayerCharacter` | `ABaseCharacter` | 玩家角色，配置玩家专属移动参数 | `UFacingComponent`, `USkillSystemComponent` |
| `AEnemyCharacter` | `ABaseCharacter` | 敌人角色，配置 AI 移动参数 | （暂无额外组件） |

**ABaseCharacter 关键成员：**
- `MoveToLocation(FVector)` — NavMesh 寻路移动（SimpleMoveToLocation）
- `StopMovement()` — 停止移动
- `GetSpeed()` — 当前移动速度
- 速度参数：`WalkSpeed=300`, `RunSpeed=600`, `LockedWalkSpeed=150`, `LockedRunSpeed=300`
- 注视参数：`LockOnRange=1000`
- `StopDistance` 已移除（攻击距离由技能 `MaxSkillRange` 控制）

### 2.2 控制器

| 类名 | 父类 | 作用 |
|------|------|------|
| `AWorldPlayerController` | `APlayerController` | 处理鼠标输入（右键全功能操作） |

**输入映射：**
- 右键 → `OnRightMouseClick()` → 根据 `GetNextSkillCategory()` 分类处理
- 左键 → `OnLeftMouseClick()` 已闲置（移除绑定即可）

**关键成员：**
| 成员 | 类型 | 作用 |
|------|------|------|
| `LastClickTarget` | `FVector` | 最后一次右键点击位置（供跳跃技能使用） |
| `bPendingAttack` | `bool` | 是否等待移动到攻击距离后自动攻击 |
| `PendingMaxRange` | `float` | 待攻击的最大距离值 |

### 2.3 组件体系

| 组件 | 挂载位置 | 作用 |
|------|---------|------|
| `UAttributeComponent` | 角色 | 血量、法力、攻击属性、防御属性管理 |
| `UDamageCalculatorComponent` | 角色 | 计算最终伤害（含暴击、护甲减免） |
| `UFacingComponent` | 玩家角色 | 行走/注视两种朝向模式切换 |
| `UClickDetectionComponent` | PlayerController | 屏幕鼠标射线检测 |
| `USkillSystemComponent` | 角色 | 技能组管理、阶段状态机 |
| `UCameraControllerComponent` | PlayerController | 双轴独立弹性相机跟随 |

---

## 三、系统详解

### 3.0 玩家状态系统

#### 3.0.1 状态枚举

```cpp
UENUM(BlueprintType)
enum class EPlayerState : uint8
{
    Default UMETA(DisplayName = "默认状态"),   // 默认行走/奔跑
    Battle  UMETA(DisplayName = "战斗状态")    // 锁定敌人，技能循环
};
```

#### 3.0.2 状态转移

```
Default ──[右键点击敌人]──→ Battle
Battle  ──[敌人超出战斗感知范围 或 目标丢失]──→ Default
```

#### 3.0.3 各状态行为

**默认状态：**
| 操作 | 行为 |
|------|------|
| 右键点击地面 | MoveToLocation，速度 300（行走） |
| Shift + 右键点击地面 | MoveToLocation，速度 600（奔跑） |
| 右键点击敌人 | 锁定敌人 → Battle 状态，移向敌人（Shift 奔跑/默认行走），到技能范围后自动攻击 |

**战斗状态：**
| 操作 | 行为 |
|------|------|
| 右键点击敌人 | 循环释放技能（复用原有 `ActivateNextSkill`） |
| 右键点击地面（默认） | MoveToLocation，保持 Aiming 模式（面朝敌人），速度 300 |
| Shift + 右键点击地面 | MoveToLocation，临时 Walking 模式（面朝移动方向），速度 600；到达后恢复 Aiming 模式面朝敌人 |
| 位移/复合技能点击地面 | 直接执行技能，结束后自动恢复 Aiming 模式面朝敌人 |

#### 3.0.4 战斗感知范围
- `PlayerCharacter.BattlePerceptionRange`（蓝图可配置，默认 1500cm）
- 战斗中玩家与锁定敌人的距离超出此值 → 自动切回默认状态，清除注视目标

#### 3.0.5 相关文件
- `WorldPlayerController.h/.cpp`：状态管理、右键点击分发
- `PlayerCharacter.h`：`BattlePerceptionRange` 参数
- `FacingComponent.cpp`：移除距离自动清除注视（由 Controller 统一管理）

### 3.1 技能系统

#### 3.1.1 核心概念

```
技能生命周期（三段式）：
[Execute] ───前摇(Windup)──→ [OnExecute] ───后摇(Recovery)──→ [LinkWindow] ──超时→ [Idle]
    ↑                           ↑ 激发瞬间                  ↑
  点击触发                    伤害/效果触发              衔接等待
 不可打断                    前摇不可打断              可点击推进到下一技能
                              后摇可打断打断           超时重置索引到 0
```

#### 3.1.2 核心组件

- **`ESkillCategory`**：技能分类枚举（Attack/Movement/Utility/Hybrid），决定控制器响应行为
- **`USkillBase`**：技能基类，定义前摇/后摇/衔接时间参数和生命周期虚函数
- **`UDamageSkillBase`**：伤害技能中间类，持有 `ApplyDamage()` 方法
- **`USkillSystemComponent`**：技能系统核心，管理技能组和阶段状态机
- **`UMeleeSlashSkill`**：扇形斩击，`OnExecute()` 触发范围伤害
- **`UJumpSkill`**：跳跃技能，前摇=抛物线飞行，后摇=落地收尾

#### 3.1.3 USkillBase

| 属性 | 类型 | 默认值 | 蓝图 DisplayName | 说明 |
|------|------|--------|-----------------|------|
| `SkillName` | `FName` | — | — | 技能名称 |
| `WindupTime` | `float` | 0.3 | **前摇** | 技能起手阶段时长（秒），不可打断 |
| `RecoveryTime` | `float` | 0.5 | **后摇** | 技能收尾阶段时长（秒），可被打断 |
| `CustomLinkTime` | `float` | 0.2 | **衔接时间** | 后摇结束后额外等待时长，超时重置索引 |
| `MaxSkillRange` | `float` | 100.0 | **最大释放技能距离** | -1=无限制（远程/位移）；
| `SkillCategory` | `ESkillCategory` | Attack | — | 技能分类 |
| `SkillMontage` | `UAnimMontage*` | nullptr | — | 技能蒙太奇动画 |
| `MontageSlotName` | `FName` | NAME_None | — | 蒙太奇槽位名称 |

| 虚函数 | 调用时机 | 说明 |
|--------|---------|------|
| `Execute(Instigator)` | 前摇开始时 | 播放蒙太奇、初始化状态 |
| `OnWindupUpdate(Instigator, DeltaTime)` | 前摇每帧 | 抛物线位移、蓄力等持续逻辑 |
| `OnExecute(Instigator)` | 前摇→后摇切换帧 | 激发瞬间，伤害/落地 |
| `OnRecoveryUpdate(Instigator, DeltaTime)` | 后摇每帧 | 收尾动画更新 |
| `OnInterrupt(Instigator)` | 后摇被点击打断时 | 清理运行时状态 |

**PlaySkillMontage(Instigator) 流程：**
1. 验证 Instigator 和 SkillMontage
2. 获取 Character → GetMesh() → GetAnimInstance()
3. 调用 `Montage_Stop(0.2f, SkillMontage)`
4. 调用 `Montage_Play(SkillMontage, 1.0f)`

#### 3.1.4 UDamageSkillBase

```cpp
// 移除
float DamageAt;  // 不再需要，伤害由 OnExecute() 在激发瞬间触发

// 保留
virtual void ApplyDamage(AActor* Instigator);
```

#### 3.1.5 UMeleeSlashSkill

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `HalfAngleDeg` | 22.5 | 扇形半角（度） |
| `BaseDamage` | 5.0 | 技能基础伤害 |
| `MaxZDiff` | 150.0 | 最大高度差 |

**生命周期：**
- `Execute()` → 停止移动 → 播放蒙太奇
- `OnExecute()` → `ApplyDamage()`（扇形范围 SphereOverlap + 角度过滤 + 伤害计算）
- 后摇期间可被点击打断

> `Radius` 已移除，扇形检测半径直接使用 `MaxSkillRange`

#### 3.1.6 UJumpSkill

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `JumpRange` | 500.0 | 最大跳跃距离（厘米） |
| `JumpHeight` | 200.0 | 抛物线最高点（厘米） |

**生命周期（不可打断=前摇, 可打断=后摇）：**
- **前摇（WindupTime=0.72s）**：抛物线位移，`OnWindupUpdate()` Tick 驱动
- **技能触发**：`OnExecute()` → 落地，通知摄像机恢复
- **后摇（RecoveryTime=1.35s）**：落地收尾动画，可被玩家打断
- **衔接时间（CustomLinkTime=0.2s）**：超时后索引重置

**Execute 流程：**
1. 从 `WorldPlayerController::GetLastClickTarget()` 读取目标位置
2. 超过 `JumpRange` 则截断
3. NavMesh 检测是否可达 → 不可达则原地起跳
4. 停止移动 → 播放蒙太奇 → 初始化跳跃状态
5. `OnWindupUpdate()` 每帧按抛物线 `4 * JumpHeight * t * (1-t)` 计算位置
6. 碰撞检测 → 撞到障碍物提前落地
7. 前摇结束 → `OnExecute()` → 落地 → 进入后摇

> `FlyDuration` 移除，由 `WindupTime` 替代。`GetInterruptibleAt()` 移除，前摇天然不可打断。

#### 3.1.7 USkillSystemComponent 状态机

**技能阶段枚举：**

```cpp
enum class ESkillPhase : uint8
{
    Idle,       // 空闲
    Windup,     // 前摇（不可打断）
    Recovery,   // 后摇（可打断）
    LinkWindow  // 衔接等待（可点击推进到下一技能，超时重置索引）
};
```

**状态流转：**

```
                       ActivateNextSkill()
                             │
                             ▼
  ┌────────┐  点击   ┌──────────┐
  │  Idle  │───────→│  Windup   │ WindupTime 到 → OnExecute → Recovery
  └────────┘        └──────────┘
       ▲                  │
       │                  ▼
       │           ┌──────────┐
       │           │ Recovery  │── 玩家点击 → OnInterrupt → 下一技能 Windup
       │           └────┬─────┘
       │                │ RecoveryTime 到
       │                ▼
       │           ┌──────────┐
       │           │LinkWindow│── 玩家点击 → 下一技能 Windup
       │           └────┬─────┘
       │                │ CachedLinkDuration 超时
       │                ▼
       └───────── GroupSkillIndex = 0 ──→ Idle
```

**ActivateNextSkill() 逻辑：**

```
switch SkillPhase:
    Windup     → return（前摇不可打断）
    Recovery   → OnInterrupt() → 清当前技能 → 继续
    Idle       → 继续
    LinkWindow → 继续

组内取下一技能（1 技能时索引永远指向自己）
CurrentSkill = Skill, Phase = Windup, PhaseStartTime = Now
Skill->Execute(Owner)
if SkillGroup.Num() > 1 → GroupSkillIndex 推进
```

#### 3.1.8 技能组

```cpp
// 当前唯一的技能组（预留多组扩展）
UPROPERTY(EditDefaultsOnly, Instanced, Category = "Skills")
TArray<TObjectPtr<USkillBase>> SkillGroup;

int32 GroupSkillIndex = 0;     // 组内技能索引
int32 CurrentGroupIndex = 0;   // 当前技能组索引（预留）
```

**索引推进规则：**

| 场景 | 行为 |
|------|------|
| 1 技能组 | `GroupSkillIndex` 始终为 0（可自打断自循环） |
| 多技能组 + 衔接时间内 | 点击推进到下一技能 |
| 多技能组 + 衔接超时 | `GroupSkillIndex = 0` 重置到第一个 |

> 衔接时间 = `RecoveryTime + CustomLinkTime`，组件在 Recovery 结束时缓存 `CachedLinkDuration`

### 3.2 输入系统

#### 3.2.1 鼠标右键（全功能）

```
右键点击（IA_RightClick Triggered）
  ↓
ClickDetectionComponent->DetectMouseClick(false)
  ↓
[点击敌人]
  ├─ 获取 NextSkillCategory
  ├─ Movement/Utility → ActivateNextSkill()
  └─ Attack/Hybrid → 距离检查
      ├─ 在范围内 → ActivateNextSkill()
      └─ 超出范围 → bPendingAttack + MoveToLocation

[点击地面]
  ├─ 获取 NextSkillCategory
  ├─ Movement/Hybrid/Utility → ActivateNextSkill()
  └─ 其他 → MoveToLocation()
```

#### 3.2.2 鼠标左键

已闲置，可由蓝图解除绑定。

### 3.3 朝向系统

- Walking 模式：面朝移动方向（`bOrientRotationToMovement=true`）
- Aiming 模式：面朝锁定目标（`FacingComponent` 控制旋转）
- 超过 `LockOnRange(1000cm)` 自动切回行走

### 3.4 属性与伤害系统

| 分类 | 属性 | 默认值 |
|------|------|--------|
| 血量 | MaxHealth / Health | 100 / 100 |
| 法力 | MaxMana / Mana | 50 / 50 |
| 攻击 | BaseDamage / CritRate / CritMultiplier | 50 / 0.3 / 2.0 |
| 防御 | Armor / DamageReduction | 20 / 0.1 |

伤害计算：`RawDamage → 暴击判定 → 护甲减免 → 伤害减免 → FinalDamage`

---

## 四、数据流

### 右键攻击处理流程

```
右键点击
  │
  ├─ 控制器判断 NextSkillCategory
  │   ├─ Movement/Utility → ActivateNextSkill()
  │   └─ Attack → 距离检查
  │
  ▼
ActivateNextSkill()
  ├─ Windup → return
  ├─ Recovery → OnInterrupt()，继续
  ├─ Idle/LinkWindow → 继续
  │
  ▼
Skill->Execute(Owner) → Windup 开始
  │
  ▼
TickComponent (Windup)
  ├─ OnWindupUpdate() → 抛物线/蓄力
  └─ WindupTime 到 → OnExecute() → 伤害/落地 → Recovery
  │
  ▼
TickComponent (Recovery)
  ├─ OnRecoveryUpdate() → 收尾动画
  ├─ 右键点击 → OnInterrupt() → 下一技能 Windup
  └─ RecoveryTime 到 → LinkWindow
  │
  ▼
LinkWindow
  ├─ 右键点击 → 下一技能 Windup
  └─ CachedLinkDuration 超时 → GroupSkillIndex=0 → Idle
```

---

## 五、蓝图配置指引

### 5.1 技能配置

在角色蓝图中：
1. 选中 `SkillSystemComponent`
2. 在 `Skills > SkillGroup` 中添加技能实例
3. 配置每个技能实例的参数

**近战技能示例（前摇→技能触发→后摇）：**

| 参数 | DisplayName | 值 | 说明 |
|------|------------|-----|------|
| SkillName | — | "出拳" | |
| **前摇** | 前摇 | 0.3 | 出拳起手时间 |
| **后摇** | 后摇 | 0.7 | 收拳时间，可打断 |
| **衔接时间** | 衔接时间 | 0.2 | 后摇后等待时间，超时重置 |
| 最大释放技能距离 | 最大释放技能距离 | 100 | 1 米攻击距离 |
| SkillCategory | — | Attack | |
| SkillMontage | — | [拖入蒙太奇] | |

**跳跃技能示例：**

| 参数 | DisplayName | 值 | 说明 |
|------|------------|-----|------|
| SkillName | — | "跳跃" | |
| **前摇** | 前摇 | 0.72 | 抛物线飞行时间 |
| **后摇** | 后摇 | 1.35 | 落地收尾时间 |
| **衔接时间** | 衔接时间 | 0.2 | 落地后等待 |
| 最大释放技能距离 | 最大释放技能距离 | -1 | 不受距离限制 |
| SkillCategory | — | Movement | 位移技能 |
| JumpRange | — | 500 | 最远跳 5 米 |
| JumpHeight | — | 200 | 最高 2 米 |

---

## 六、开发日志

| 日期 | 修改内容 | 涉及文件 |
|------|---------|---------|
| 06/15 | **[新增] 玩家行为状态系统**：EPlayerState(Default/Battle) + Shift奔跑 + 战斗感知范围 | WorldPlayerController.h/.cpp, PlayerCharacter.h/.cpp, FacingComponent.cpp |
| 06/14 | **[重构] 技能系统三段式重构**：前摇/技能触发/后摇/衔接时间，替换 Duration/DamageAt/InterruptibleAt 线性模型 | SkillBase.h/.cpp, DamageSkillBase.h/.cpp, SkillSystemComponent.h/.cpp, MeleeSlashSkill.h/.cpp, JumpSkill.h/.cpp, PlayerCharacter.cpp, SkillTypes.h |
| 06/14 | **[新增] 技能组系统 + 衔接时间 + 1 技能自循环** | SkillSystemComponent.h/.cpp |
| 06/14 | **[重命名] bIsMovementSkill → ESkillCategory** | SkillBase.h, SkillTypes.h, SkillSystemComponent.h/.cpp, WorldPlayerController.cpp, JumpSkill.cpp |
| 06/14 | **[重命名] MaxAttackRange → MaxSkillRange, Radius 移除合并** | SkillBase.h, MeleeSlashSkill.h/.cpp, SkillSystemComponent.h/.cpp, WorldPlayerController.cpp, JumpSkill.cpp, PlayerCharacter.cpp |
| 06/14 | **[清理] PlayerCharacter 移除默认技能注册，技能完全由蓝图 SkillGroup 配置** | PlayerCharacter.cpp |
| 06/14 | **[新增] 命名规范：代码标识符英文，提示/注释/DisplayName 中文** | Project_Knowledge.md |
| 06/14 | **[修复] ComboWindowDuration 空指针崩溃** | SkillSystemComponent.cpp |
| 06/13 | **[修复] 跳跃后摇期无法打断** | JumpSkill.cpp, SkillSystemComponent.cpp |
| 06/12 | 详情见旧版文档 | 多文件 |

---

## 七、已知问题 / 待优化

1. SimpleMoveToLocation 的精确度有限（约 50cm 容差），Tick 中 bPendingAttack 的到达判断使用 80cm 容差补偿
