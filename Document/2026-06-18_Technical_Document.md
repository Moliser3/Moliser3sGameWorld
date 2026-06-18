# 技术文档 — Moliser3sGameClient

> 引擎：Unreal Engine 5.7  
> 语言：C++  
> 最后更新：2026/06/18 15:00

---

## 一、项目概览

### 1.1 项目结构

```
Moliser3sGameClient/Source/
├── BaseCharacter.h/.cpp          # 基础角色类
├── PlayerCharacter.h/.cpp        # 玩家角色（状态监听者）
├── EnemyCharacter.h/.cpp         # 敌人角色
├── WorldPlayerController.h/.cpp  # 玩家控制器（状态拥有者 + 输入分发）
├── WorldGameMode.h/.cpp          # 游戏模式
├── GamePlayerState.h             # 双轴状态枚举 + 事件委托
├── Component/
│   ├── Attribute/                # 属性组件
│   ├── Damage/                   # 伤害计算组件
│   ├── Facing/                   # 朝向控制组件
│   ├── Input/                    # 点击检测组件
│   ├── Camera/                   # 摄像机控制器组件
│   └── Skill/                    # 技能系统组件（双技能组状态机）
└── Skill/
    ├── SkillTypes.h              # 枚举定义 + FSkillStage 结构体
    ├── SkillBase.h/.cpp          # 技能基类（Stages 数组，多阶段）
    ├── DamageSkillBase.h/.cpp    # 伤害技能中间类
    ├── MeleeSlashSkill.h/.cpp    # 扇形斩击技能（从Stage读取伤害参数）
    └── JumpSkill.h/.cpp          # 跳跃技能（抛物线位移，Stages[0]）
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

#### 3.0.1 双轴状态枚举

```cpp
// 战斗感知轴
UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Default          UMETA(DisplayName = "默认"),
    BattlePerception UMETA(DisplayName = "战斗感知")
};

// 行为轴
UENUM(BlueprintType)
enum class EActionState : uint8
{
    Idle    UMETA(DisplayName = "闲置"),
    Walking UMETA(DisplayName = "行走"),
    Running UMETA(DisplayName = "奔跑"),
    Skill   UMETA(DisplayName = "释放技能")
};
```

#### 3.0.2 组合行为矩阵

| Combat | Action | 面朝 | 速度 | 说明 |
|--------|--------|------|------|------|
| Default | Idle | — | 0 | 初始/停止 |
| Default | Walking | 移动方向 | 300 | 普通行走 |
| Default | Running | 移动方向 | 600 | Shift奔跑 |
| Default | Skill | 点击方向 | 0 | 释放技能 |
| Battle | Idle | 目标 | 0 | 锁定敌人，待命 |
| Battle | Walking | 目标 | 300 | 面朝敌人行走 |
| Battle | Running | 移动方向 | 600 | 面朝移动方向奔跑 |
| Battle | Skill | 目标/点击方向 | 0 | 释放技能 |

#### 3.0.3 状态转移

```
Combat:
  Default ──[左/右键点击敌人]──→ BattlePerception
  BattlePerception ──[超距且非Running]──→ Default

Action:
  Idle ──[左键地面/技能]──→ Walking/Running/Skill
  Walking ──[再次点击]──→ Skill
  Running ──[VelSq<100 + 延时>0.3s]──→ Idle
  Skill ──[技能完成]──→ Idle
```

#### 3.0.4 事件回调链

```
WorldPlayerController (状态拥有者)
  ├── OnCombatStateChanged(ECombatState)
  │   └── PlayerCharacter → 清除/保留目标引用
  └── OnActionStateChanged(EActionState)
      └── PlayerCharacter → 设置速度 + 面朝模式

PlayerCharacter (状态监听者)
  ├── OnCombatStateChanged
  │   └── Default → PendingAimTarget = nullptr
  └── OnActionStateChanged
      ├── Idle   → 若Battle+Pending有目标 → 恢复Aiming
      ├── Walking→ 若Battle → 面朝目标; 否则面朝移动方向
      ├── Running→ 速度600, 面朝移动方向
      └── Skill  → StopMovement
```

#### 3.0.5 战斗感知范围
- `PlayerCharacter.BattlePerceptionRange`（蓝图可配置，默认 1500cm）
- 战斗中玩家与锁定敌人的距离超出此值且非奔跑中 → 自动切回默认状态

#### 3.0.6 Alt 防御
- `OnAltPressed()`：设置 `bDefending = true`
- `OnAltReleased()`：设置 `bDefending = false`
- 蓝图输入事件绑定 Alt 键

#### 3.0.7 相关文件
- `GamePlayerState.h`：状态枚举 + 事件委托定义（新增）
- `WorldPlayerController.h/.cpp`：状态拥有者、状态转移、事件广播、左/右键分发、Alt防御
- `PlayerCharacter.h/.cpp`：状态监听者、速度/面朝逻辑
- `FacingComponent.cpp`：独立管理 Aiming/Walking 模式

### 3.1 技能系统

#### 3.1.1 核心概念

```
技能多阶段生命周期：
每个技能有 N 个阶段（FSkillStage），阶段推进按技能类型（ESkillType）连续出现次数

组索引推进（每次点击+1）：
[A][A][A][B][B][A][B]
A_0 A_1 A_2 B_0 B_1 A_0 B_0

阶段推进规则：
  当前 SkillType == 上一个 SkillType → StageForType++
  当前 SkillType != 上一个 SkillType → StageForType = 0
```

#### 3.1.2 核心组件

- **`ESkillType`**：技能类型枚举（None/直拳/勾拳等），用于阶段追踪和对比
- **`FSkillStage`**：技能阶段结构体，包含前摇/后摇/衔接时间/蒙太奇/伤害参数/扇形角/高度差
- **`USkillBase`**：技能基类，持有 `TArray<FSkillStage> Stages` 数组
- **`UDamageSkillBase`**：伤害技能中间类，持有 `ApplyDamage()` 方法
- **`USkillSystemComponent`**：双技能组（Left/Right），各自独立索引和阶段追踪
- **`UMeleeSlashSkill`**：扇形斩击，`OnExecute()` 触发，从 `Stages[CurrentStage]` 读取伤害参数
- **`UJumpSkill`**：跳跃技能，阶段参数在 `Stages[0]` 中

#### 3.1.3 USkillBase

| 属性 | 类型 | DisplayName | 说明 |
|------|------|-------------|------|
| `SkillName` | `FName` | 技能名称 | 技能名称标识 |
| `SkillType` | `ESkillType` | 技能类型 | 系统据此追踪阶段 |
| `MaxSkillRange` | `float` | 最大释放技能距离 | -1=无限制 |
| `SkillCategory` | `ESkillCategory` | 技能分类 | Attack/Movement/Utility/Hybrid |
| `Stages` | `TArray<FSkillStage>` | 技能阶段列表 | 每个阶段独立配置 |

| 虚函数 | 调用时机 | 说明 |
|--------|---------|------|
| `Execute(Instigator)` | 前摇开始时 | 播放当前阶段蒙太奇、初始化状态 |
| `OnWindupUpdate(Instigator, DeltaTime)` | 前摇每帧 | 抛物线位移、蓄力等 |
| `OnExecute(Instigator)` | 前摇→后摇切换帧 | 激发瞬间，伤害/落地 |
| `OnRecoveryUpdate(Instigator, DeltaTime)` | 后摇每帧 | 收尾动画更新 |
| `OnInterrupt(Instigator)` | 后摇被点击打断时 | 清理运行时状态 |

**FSkillStage 结构体字段：**

| 字段 | 类型 | DisplayName | 说明 |
|------|------|-------------|------|
| `WindupTime` | float | 前摇 | 阶段起手时间 |
| `RecoveryTime` | float | 后摇 | 阶段收尾时间 |
| `CustomLinkTime` | float | 衔接时间 | 后摇后额外等待 |
| `SkillMontage` | UAnimMontage* | 蒙太奇 | 阶段动画 |
| `MontageSlotName` | FName | 蒙太奇槽位 | 槽位名称 |
| `BaseDamage` | float | 基础伤害 | 阶段伤害 |
| `HalfAngleDeg` | float | 扇形半角 | 扇形检测角度 |
| `MaxZDiff` | float | 最大高度差 | 高度容差 |

**PlaySkillMontage(Instigator) 流程：**
1. 验证 Instigator 和 `Stages[CurrentStage].SkillMontage`
2. 获取 Character → GetMesh() → GetAnimInstance()
3. 调用 `Montage_Stop(0.2f)` — 停止所有蒙太奇
4. 调用 `Montage_Play(SkillMontage, 1.0f)`

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
- **前摇（WindupTime=0.72s，Stages[0]）**：抛物线位移，`OnWindupUpdate()` Tick 驱动
- **技能触发**：`OnExecute()` → 落地，通知摄像机恢复
- **后摇（RecoveryTime=1.35s，Stages[0]）**：落地收尾动画，可被玩家打断
- **衔接时间（CustomLinkTime=0.2s，Stages[0]）**：超时后索引重置

**Execute 流程：**
1. 从 `WorldPlayerController::GetLastClickTarget()` 读取目标位置
2. 超过 `JumpRange` 则截断
3. NavMesh 检测是否可达 → 不可达则原地起跳
4. 停止移动 → 播放蒙太奇 → 初始化跳跃状态
5. `OnWindupUpdate()` 每帧按抛物线 `4 * JumpHeight * t * (1-t)` 计算位置
6. 碰撞检测 → 撞到障碍物提前落地
7. 前摇结束 → `OnExecute()` → 落地 → 进入后摇

> 阶段参数（WindupTime/RecoveryTime/CustomLinkTime）已移入 `Stages[0]`。

#### 3.1.7 USkillSystemComponent 状态机

**技能阶段枚举：**

```cpp
enum class ESkillPhase : uint8
{
    Idle,       // 空闲
    Windup,     // 前摇（不可打断）
    Recovery,   // 后摇（可打断）
    LinkWindow  // 衔接等待
};
```

**状态流转：**

```
                    ActivateLeft() / ActivateRight()
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
       └────── 重置左右组阶段追踪归零 ──→ Idle
```

**双技能组：**

```cpp
// 左键技能组（普通攻击）
UPROPERTY(EditDefaultsOnly, Instanced, Category = "LeftSkill")
TArray<TObjectPtr<USkillBase>> LeftSkillGroup;

// 右键技能组
UPROPERTY(EditDefaultsOnly, Instanced, Category = "RightSkill")
TArray<TObjectPtr<USkillBase>> RightSkillGroup;

int32 LeftGroupIndex = 0;     // 左组索引
int32 RightGroupIndex = 0;    // 右组索引

ESkillType LeftLastSkillType;  // 左组上一技能类型
int32 LeftStageForType;        // 左组当前技能类型阶段进度
// 右组同理
```

**阶段推进规则（ExecuteSkillFromGroup 核心逻辑）：**

```
switch SkillPhase:
    Windup     → return（前摇不可打断）
    Recovery   → OnInterrupt() → 清当前技能 → 继续
    Idle/LinkWindow → 继续

组内取 Group[Index]
判断阶段：
  Skill->SkillType == LastSkillType → StageForType++
  Skill->SkillType != LastSkillType → StageForType = 0
LastSkillType = Skill->SkillType

Skill->SetCurrentStage(StageForType)
Skill->Execute(Owner)
Index = (Index + 1) % Group.Num()   // 组索引每次推进
```

**衔接超时行为：**
- LeftGroupIndex/RightGroupIndex 归零
- LeftStageForType/RightStageForType 归零
- LeftLastSkillType/RightLastSkillType 重置为 None

### 3.2 输入系统

#### 3.2.1 鼠标左键

```
左键点击（IA_LeftClick Triggered）
  ↓
ClickDetectionComponent->DetectMouseClick(false)
  ↓
[点击敌人]
  ├─ SetAimTarget + Enter Battle
  ├─ 检查左键技能 MaxSkillRange
  ├─ 超距 → bPendingAttack + MoveToLocation
  └─ 在范围 → ActivateLeft()

[点击地面]
  ├─ 战斗状态 + Shift → 奔跑，面朝移动方向，到达后恢复注视
  ├─ 战斗状态 + 无 Shift → 行走，面朝敌人
  ├─ 默认状态 + Shift → 奔跑
  └─ 默认状态 + 无 Shift → 行走
```

#### 3.2.2 鼠标右键

```
右键点击（IA_RightClick Triggered）
  ↓
[点击敌人] → SetAimTarget + Enter Battle
  ↓
直接 ActivateRight() — 无距离检查，方向指向点击位置

[点击地面] → 记录 LastClickTarget
  ↓
直接 ActivateRight()
```

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

### 左键攻击处理流程

```
左键点击敌人
  │
  ├─ SetAimTarget + Enter Battle
  ├─ GetMaxSkillRange() 距离检查
  │   ├─ 超距 → bPendingAttack + MoveToLocation
  │   └─ 在范围 → ActivateLeft()
  │
  ▼
ActivateLeft()
  ├─ Windup → return
  ├─ Recovery → OnInterrupt()
  ├─ Idle/LinkWindow → 继续
  │
  ▼
ExecuteSkillFromGroup(LeftSkillGroup, LeftGroupIndex, LeftLastType, LeftStageForType)
  ├─ 取 LeftGroup[LeftGroupIndex]
  ├─ Stage判定（连续同类型推进）
  ├─ Skill->SetCurrentStage(StageForType)
  └─ Skill->Execute(Owner) → Windup 开始
```

### 右键释放流程

```
右键点击
  │
  ├─ 点击敌人 → SetAimTarget + Enter Battle
  └─ 记录 LastClickTarget 为方向
  │
  ▼
ActivateRight()
  └─ ExecuteSkillFromGroup(RightSkillGroup, RightGroupIndex, RightLastType, RightStageForType)
      ├─ 取 RightGroup[RightGroupIndex]
      ├─ 无距离检查（立即释放）
      ├─ Stage判定（连续同类型推进）
      ├─ Skill->SetCurrentStage(StageForType)
      └─ Skill->Execute(Owner) → Windup 开始
```

### 技能执行生命周期（共享）

```
Skill->Execute(Owner) → Windup 开始
  │
  ▼
TickComponent (Windup)
  ├─ OnWindupUpdate() → 抛物线/蓄力
  └─ GetWindupTime() 到 → OnExecute() → 伤害/落地 → Recovery
  │
  ▼
TickComponent (Recovery)
  ├─ OnRecoveryUpdate() → 收尾动画
  ├─ 点击 → OnInterrupt() → 下一技能 Windup
  └─ GetRecoveryTime() 到 → LinkWindow
  │
  ▼
LinkWindow
  ├─ 点击 → 下一技能 Windup
  └─ CachedLinkDuration 超时 → 重置左右组追踪 → Idle
```

---

## 五、蓝图配置指引

### 5.1 双技能组配置

在角色蓝图中：
1. 选中 `SkillSystemComponent`
2. 在 `LeftSkill` > `LeftSkillGroup` 中添加左键技能实例
3. 在 `RightSkill` > `RightSkillGroup` 中添加右键技能实例
4. 配置每个技能实例的 `Stages` 数组

**近战技能阶段配置（直拳/勾拳）：**

| 字段 | DisplayName | 说明 |
|------|------------|------|
| SkillName | 技能名称 | "直拳" |
| SkillType | 技能类型 | 设置为对应枚举（StraightPunch/Hook） |
| MaxSkillRange | 最大释放技能距离 | 攻击判定距离 |
| SkillCategory | 技能分类 | Attack |
| Stages[0].WindupTime | 前摇 | 阶段0起手时间 |
| Stages[0].RecoveryTime | 后摇 | 阶段0收尾时间 |
| Stages[0].SkillMontage | 蒙太奇 | 阶段0动画 |
| Stages[0].BaseDamage | 基础伤害 | 阶段0伤害 |
| Stages[0].HalfAngleDeg | 扇形半角 | 阶段0扇形角度 |

**跳跃技能示例（Stages[0]）：**

| 字段 | DisplayName | 值 | 说明 |
|------|------------|-----|------|
| SkillName | 技能名称 | "跳跃" | |
| SkillType | 技能类型 | None（暂未分类） | |
| MaxSkillRange | 最大释放技能距离 | -1 | 不受距离限制 |
| SkillCategory | 技能分类 | Movement | 位移技能 |
| Stages[0].WindupTime | 前摇 | 0.72 | 抛物线飞行时间 |
| Stages[0].RecoveryTime | 后摇 | 1.35 | 落地收尾时间 |
| Stages[0].CustomLinkTime | 衔接时间 | 0.2 | 落地后等待 |
| JumpRange | 最大跳跃距离 | 500 | 最远跳 5 米 |
| JumpHeight | 跳跃最高点高度 | 200 | 最高 2 米 |

---

## 六、开发日志

| 日期 | 修改内容 | 涉及文件 |
|------|---------|---------|
| 06/18 | **[重构] 正交状态机**：双轴状态(ECombatState+EActionState) + 事件驱动 + 替换EPlayerState | GamePlayerState.h, WorldPlayerController.h/.cpp, PlayerCharacter.h/.cpp |
| 06/15 | **[重构] 多阶段技能系统 + 双技能组 + 左键恢复 + 变量清理** | 所有技能/组件/控制器文件 |
| 06/15 | **[新增] 多阶段技能**：FSkillStage + ESkillType + Stages数组，阶段按技能类型连续出现次数推进 | SkillTypes.h, SkillBase.h/.cpp, MeleeSlashSkill.h/.cpp, JumpSkill.h/.cpp |
| 06/15 | **[新增] 双技能组**：LeftSkillGroup/RightSkillGroup，各自独立索引和阶段追踪 | SkillSystemComponent.h/.cpp |
| 06/15 | **[新增] 鼠标左键恢复**：OnLeftMouseClick实现，点击地面移动，点击敌人攻击 | WorldPlayerController.h/.cpp |
| 06/15 | **[变更] 右键简化**：无距离检查，直接释放技能 | WorldPlayerController.cpp |
| 06/15 | **[新增] Alt防御**：OnAltPressed/OnAltReleased + bDefending | WorldPlayerController.h/.cpp |
| 06/15 | **[清理] 变量清理**：移除LockedWalkSpeed/LockedRunSpeed/LockOnRange/bAimingFullSpeed | BaseCharacter.h, PlayerCharacter.h/.cpp |
| 06/15 | **[修复] PlaySkillMontage**：Montage_Stop去掉参数，停止所有蒙太奇 | SkillBase.cpp |
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
