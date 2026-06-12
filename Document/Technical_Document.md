# 技术文档 — Moliser3sGameClient

> 引擎：Unreal Engine 5.7  
> 语言：C++  
> 最后更新：2026/06/12

---

## 一、项目概览

### 1.1 项目结构

```
Moliser3sGameClient/Source/
├── BaseCharacter.h/.cpp          # 基础角色类
├── PlayerCharacter.h/.cpp        # 玩家角色
├── EnemyCharacter.h/.cpp         # 敌人角色
├── WorldPlayerController.h/.cpp  # 玩家控制器（右键全功能）
├── WorldGameMode.h/.cpp          # 游戏模式
├── Component/
│   ├── Attribute/                # 属性组件
│   ├── Damage/                   # 伤害计算组件
│   ├── Facing/                   # 朝向控制组件
│   ├── Input/                    # 点击检测组件
│   └── Skill/                    # 技能系统组件
└── Skill/
    ├── SkillBase.h/.cpp          # 技能基类
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
- `StopDistance` 已移除（攻击距离由技能 MaxAttackRange 控制）

### 2.2 控制器

| 类名 | 父类 | 作用 |
|------|------|------|
| `AWorldPlayerController` | `APlayerController` | 处理鼠标输入（右键全功能操作） |

**输入映射：**
- 右键 → `OnRightMouseClick()` → 三合一：移动/注视/攻击
- 左键 → `OnLeftMouseClick()` 已闲置（移除绑定即可）

**新增关键成员：**
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
| `USkillSystemComponent` | 角色 | 技能列表管理、技能队列循环、连招窗口、打断机制 |

---

## 三、系统详解

### 3.1 技能系统

#### 3.1.1 核心组件

- **`USkillSystemComponent`**：技能系统的核心，挂载在角色上
- **`USkillBase`**：技能基类，所有技能继承此类
- **`UMeleeSlashSkill`**：扇形斩击技能
- **`UJumpSkill`**：跳跃技能（抛物线位移）

#### 3.1.2 USkillSystemComponent 关键变量

| 变量 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `SkillList` | `TArray<USkillBase*>` | 空 | 编辑器中配置的所有技能 |
| `SkillQueue` | `TArray<USkillBase*>` | 空 | 运行时循环队列，自动从 SkillList 填充 |
| `QueueIndex` | `int32` | 0 | 队列当前索引 |
| `CurrentSkill` | `USkillBase*` | nullptr | 当前正在播放的技能 |
| `CurrentSkillStartTime` | `float` | 0.0 | 当前技能开始时间戳 |
| `bSkillActive` | `bool` | false | 技能是否正在 ACTIVE 状态 |
| `bDamageApplied` | `bool` | false | 当前技能的伤害是否已延迟应用 |
| `ComboWindowDuration` | `float` | 0.5 | 连招窗口持续时间（秒） |
| `bInComboWindow` | `bool` | false | 是否处于连招缓冲期 |
| `ComboWindowEndTime` | `float` | 0.0 | 连招窗口结束时间戳 |

#### 3.1.3 核心函数

| 函数 | 参数 | 作用 |
|------|------|------|
| `ActivateNextSkill()` | 无 | 按状态机逻辑释放队列中的下一个技能（含打断检测） |
| `AddSkill(NewSkill)` | `USkillBase*` | 添加技能到 SkillList |
| `SetSkillQueue(InQueue)` | `TArray<USkillBase*>` | 设置技能循环队列 |
| `IsSkillActive()` | 无 | 返回是否正在 ACTIVE 或 COMBO_WINDOW 状态 |
| `GetMaxAttackRange()` | 无 | 遍历队列返回第一个近战技能的 MaxAttackRange（-1 表示全远程） |
| `TickComponent(DeltaTime, ...)` | — | 检测 DamageAt/Duration/连招窗口到期 |

#### 3.1.4 状态机（核心逻辑）

```
右键点击敌人
  │
  ▼
OnRightMouseClick
  ├─ 距离 ≤ MaxAttackRange → ActivateNextSkill()
  ├─ 距离 > MaxAttackRange → MoveTo + bPendingAttack = true
  └─ MaxAttackRange = -1 → ActivateNextSkill()

Tick 检测 bPendingAttack
  ├─ 距离 ≤ MaxRange + 80cm → ActivateNextSkill()
  └─ 速度 ≈ 0（到达）→ ActivateNextSkill()

ActivateNextSkill() 内部：
  │
  ▼
IDLE → 执行当前技能 → ACTIVE
ACTIVE → 右键点击
  ├─ elapsed < InterruptibleAt → IGNORE（不可打断）
  └─ elapsed ≥ InterruptibleAt → INTERRUPT（打断执行下一个）
ACTIVE → Duration 到期 → COMBO_WINDOW
COMBO_WINDOW → 右键 → 下一个技能
COMBO_WINDOW → 超时(0.5s) → QueueIndex=0 → IDLE
```

#### 3.1.5 USkillBase 关键属性

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `SkillName` | `FName` | — | 技能名称 |
| `Duration` | `float` | 1.0 | 技能持续时间（秒） |
| `DamageAt` | `float` | 0.3 | 伤害触发时间点（秒），动画的"命中帧" |
| `InterruptibleAt` | `float` | 0.3 | 可打断时间点（秒），超过可提前释放下一个技能；0=全程可打断 |
| `MaxAttackRange` | `float` | 100.0 | 最大攻击距离（厘米），-1=无限制（远程） |
| `SkillMontage` | `UAnimMontage*` | nullptr | 技能蒙太奇动画 |
| `MontageSlotName` | `FName` | NAME_None | 蒙太奇槽位名称 |

**PlaySkillMontage(Instigator) 流程：**
1. 验证 Instigator 和 SkillMontage
2. 获取 Character → GetMesh() → GetAnimInstance()
3. 调用 `Montage_Stop(0.2f, SkillMontage)`
4. 调用 `Montage_Play(SkillMontage, 1.0f)`

**Execute vs ApplyDamage：**
- `Execute()`：停止移动 + 播放蒙太奇（跳跃技能额外调用 Character->Jump()）
- `ApplyDamage()`：在 `DamageAt` 时间点由 SkillSystemComponent 的 Tick 调用

#### 3.1.6 UMeleeSlashSkill

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `Radius` | 100.0 | 攻击半径（厘米） |
| `HalfAngleDeg` | 22.5 | 扇形半角（度） |
| `BaseDamage` | 5.0 | 技能基础伤害 |
| `MaxZDiff` | 150.0 | 最大高度差 |

**Execute：** 停止移动 → 播放蒙太奇  
**ApplyDamage：** SphereOverlap → 扇形过滤 → 计算伤害

#### 3.1.7 UJumpSkill

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `JumpRange` | 500.0 | 最大跳跃距离（厘米） |
| `JumpHeight` | 200.0 | 抛物线最高点（厘米） |
| `FlyDuration` | 0.6 | 空中飞行时间（秒） |

**Execute 流程：**
1. 从 `WorldPlayerController::GetLastClickTarget()` 读取目标位置
2. 超过 JumpRange 则截断
3. NavMesh 检测是否可达 → 不可达则原地起跳
4. 停止移动 → 播放蒙太奇 → 初始化抛物线 Timer
5. Timer 每帧按 `4 * JumpHeight * t * (1-t)` 计算抛物线位置
6. 撞到障碍物或到达终点 → EndJump()

### 3.2 输入系统

#### 3.2.1 鼠标右键（全功能）

```
右键点击（IA_RightClick Triggered）
  ↓
ClickDetectionComponent->DetectMouseClick()
  ↓
命中敌人？
  是 → 距离 ≤ MaxAttackRange → ActivateNextSkill()
       距离 > MaxAttackRange → MoveTo + SetAimTarget + bPendingAttack=true
       距离 > MaxAttackRange 的 Tick 检测 → 到达后自动 ActivateNextSkill()
  否 → MoveToLocation(点击位置)
  ↓
LastClickTarget = 点击位置（供跳跃技能使用）
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

### 攻击处理流程

```
右键点击敌人
  │
  ▼
OnRightMouseClick()
  ├─ 距离 > MaxAttackRange
  │   → MoveToLocation(敌人 - Dir * MaxRange)
  │   → SetAimTarget(敌人)
  │   → bPendingAttack = true
  │   → Tick 检测: 到达距离或速度≈0
  │     → ActivateNextSkill()（进入 ACTIVE）
  │
  └─ 距离 ≤ MaxAttackRange 或 MaxAttackRange = -1
      → ActivateNextSkill()（进入 ACTIVE）
  │
  ▼
ActivateNextSkill()
  ├─ 不可打断(ACTIVE + elapsed < InterruptibleAt) → IGNORE
  ├─ 可打断(ACTIVE + elapsed ≥ InterruptibleAt) → INTERRUPT
  ├─ COMBO_WINDOW → 执行下一个
  └─ IDLE → QueueIndex=0 → 执行
  │
  ▼
Skill->Execute(Instigator)
  └─ PlaySkillMontage()
  │
  ▼
TickComponent()
  ├─ DamageAt 到 → ApplyDamage() → 伤害结算
  └─ Duration 到 → COMBO_WINDOW
```

---

## 五、蓝图配置指引

### 5.1 技能配置

在角色蓝图中：
1. 选中 `SkillSystemComponent`
2. 在 `SkillList` 中添加技能实例
3. 配置每个技能实例的参数

**近战技能示例（出拳）：**

| 参数 | 值 | 说明 |
|------|-----|------|
| SkillName | "出拳" | |
| Duration | 1.0 | 完整动画长度 |
| DamageAt | 0.25 | 拳头打中时造成伤害 |
| InterruptibleAt | 0.3 | 起手完毕后可打断 |
| MaxAttackRange | 100 | 1 米攻击距离 |
| SkillMontage | [拖入蒙太奇] | |

**跳跃技能示例：**

| 参数 | 值 | 说明 |
|------|-----|------|
| SkillName | "跳跃" | |
| Duration | 0.8 | |
| InterruptibleAt | 999 | 全程不可打断 |
| MaxAttackRange | -1 | 不使用攻击距离判定 |
| JumpRange | 500 | 最远跳 5 米 |
| JumpHeight | 200 | 最高 2 米 |
| FlyDuration | 0.6 | 空中 0.6 秒 |

### 5.2 ComboWindowDuration

在 `SkillSystemComponent` 的属性中配置，默认 0.5 秒。

---

## 六、开发日志

| 日期 | 修改内容 | 涉及文件 |
|------|---------|---------|
| 06/12 | 右键全功能操作（废弃左键、三合一移动/注视/攻击） | `WorldPlayerController.h/.cpp` |
| 06/12 | MinAttackRange 改名 MaxAttackRange，默认值 100 | `SkillBase.h` |
| 06/12 | GetNextAttackRange → GetMaxAttackRange（遍历队列） | `SkillSystemComponent.h/.cpp` |
| 06/12 | 自动攻击机制（Tick 检测 bPendingAttack） | `WorldPlayerController.h/.cpp` |
| 06/12 | StopDistance 完全移除 | `BaseCharacter.h`, `FacingComponent.cpp` |
| 06/12 | InterruptibleAt 改为蓝图可配置，默认 0.3 | `SkillBase.h` |
| 06/12 | 跳跃技能（抛物线位移，NavMesh检测，碰撞落地） | `JumpSkill.h/.cpp`, `PlayerCharacter.cpp` |
| 06/11 | 技能系统新增 DamageAt / InterruptibleAt 属性，实现延迟伤害和技能打断 | `SkillBase.h/.cpp`, `SkillSystemComponent.h/.cpp`, `MeleeSlashSkill.cpp` |
| 06/11 | 创建技术文档 | `策划案/技术文档.md` |
| 06/11 | 实现连招窗口机制（IDLE→ACTIVE→COMBO_WINDOW） | `SkillSystemComponent.h/.cpp` |
| 06/11 | 技能系统添加 ComboWindowDuration 蓝图可配置 | `SkillSystemComponent.h` |
| 06/10 | 添加蒙太奇播放功能 | `SkillBase.h/.cpp`, `MeleeSlashSkill.cpp` |
| 06/10 | 技能系统重构（循环队列、Duration 控制） | `SkillSystemComponent.h/.cpp` |
| 06/10 | 修复左键多技能 Bug | `WorldPlayerController.h/.cpp` |

---

## 七、已知问题 / 待优化

1. SimpleMoveToLocation 的精确度有限（约 50cm 容差），Tick 中 bPendingAttack 的到达判断使用 80cm 容差补偿
2. 技能打断需要蓝图中正确配置 InterruptibleAt 值（默认 0.3）
3. 跳跃技能的抛物线位移使用 Timer 驱动，若 Timer 精度不足可能出现轻微抖动