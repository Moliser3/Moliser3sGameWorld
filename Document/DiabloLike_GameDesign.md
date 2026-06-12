# 类暗黑破坏神 ARPG 游戏策划案

> 引擎：Unreal Engine 5.7  
> 语言：C++  
> 项目名称：Moliser3sGameClient  
> 策划日期：2026/06/12
> 最后更新：2026/06/12 23:46

---

## 一、游戏类型与核心体验

[内容同上，略]

---

## 二、操作与移动

### 2.1 基础操作

| 操作 | 按键 | 说明 |
|------|------|------|
| 移动 | 鼠标右键 | 点击地面 → 角色寻路移动到目标位置 |
| 攻击/释放技能 | 鼠标右键 | 点击敌人 → 近距攻击/远距移动后自动攻击 |
| 跳跃 | 右键 | 跳跃作为技能类型（UJumpSkill）融入技能循环队列，点击地面/敌人都可触发 |
| 走路/奔跑切换 | 自动 | 根据当前朝向模式自动切换速度 |

**左键已废弃**，所有操作通过鼠标右键完成。

### 2.2 移动与攻击联动

```
右键点击敌人
  ├─ 距离 ≤ MaxAttackRange → 直接攻击
  ├─ 距离 > MaxAttackRange → 走到 MaxAttackRange 处后自动攻击
  └─ MaxAttackRange = -1（远程）→ 直接攻击
```

### 2.3 跳跃（✅ 已完成，打断机制持续优化中）

- 跳跃作为技能类型（UJumpSkill）融入技能循环队列
- 右键点击地面设置目标位置，跳跃到目标（抛物线位移）
- 目标不可达时原地起跳
- 飞行中撞到障碍物则落地

**两阶段设计：**
  - **阶段1（0 ~ FlyDuration=0.72s）**：抛物线位移，由 SkillSystemComponent 的 Tick 驱动。**不可打断**（GetInterruptibleAt() 强制为 Max(InterruptibleAt, FlyDuration)=0.72）
  - **阶段2（FlyDuration ~ Duration）**：落地收尾动画，角色不再移动。**可打断**

**打断规则：** `GetInterruptibleAt() = Max(InterruptibleAt, FlyDuration)`，飞行中不可打断，收尾期可打断

**参数：** JumpRange=500, JumpHeight=200, FlyDuration=0.72, Duration=2.07

---

## 三、技能系统

### 3.1 技能循环队列

玩家通过**鼠标右键**释放技能。所有点击统一走 `ActivateNextSkill()`，不再做移动/攻击分支判断。

```
技能队列（SkillQueue）:
┌──────┬──────┬──────┬──────┐
│ 技能A │ 技能B │ 跳跃 │ 技能C │
└──┬───┴──┬───┴──┬───┴──┬───┘
   │      │      │      │
   └──▶ 循环释放，到末尾回到开头
```

`SkillQueue` 自动从蓝图 `SkillList` 填充，并在填充时**过滤空指针**。

### 3.2 连招 + 打断机制（✅ 已完成，持续优化）

```
IDLE → 右键点击 → 执行当前技能 → ACTIVE
ACTIVE → 再次右键
  ├─ elapsed < GetInterruptibleAt() → IGNORE（不可打断）
  └─ elapsed ≥ GetInterruptibleAt() → INTERRUPT（打断执行下一个）
ACTIVE → Duration 到期 → COMBO_WINDOW
COMBO_WINDOW → 右键 → 下一个技能
COMBO_WINDOW → 超时(0.5s) → QueueIndex=0 → IDLE

打断后流程：
  ├─ OnInterrupt() → 清理当前技能运行时状态
  ├─ goto ExecuteSkill → 跳过状态检查，直接执行下一个
  └─ QueueIndex 保留，不重置
```

### 3.3 技能类型

#### 类型一：扇形斩击（UMeleeSlashSkill ✅）
#### 类型二：跳跃（UJumpSkill ✅）

### 3.4 技能配置

所有参数暴露给蓝图，但 `JumpSkill` 的 `GetInterruptibleAt()` 重写为 `Max(InterruptibleAt, FlyDuration)` 防止蓝图错误覆盖飞行不可打断性。

| 属性 | 说明 |
|------|------|
| SkillName | 技能名称 |
| Duration | 技能持续时间 |
| DamageAt | 伤害触发时间点（动画的"命中帧"） |
| InterruptibleAt | 可打断时间点（子类可通过 GetInterruptibleAt() 重写） |
| MaxAttackRange | 最大攻击距离 |
| bIsMovementSkill | 是否为移动技能 |
| SkillMontage | 蒙太奇动画 |
| MontageSlotName | 槽位名称 |

---

## 四、游戏系统（⬜ 待开发）

---

## 五、任务系统（⬜ 待开发）

---

## 六、AI 与敌人系统（⬜ 待开发）

---

## 七、当前开发进度

### 已完成（✅）

| 模块 | 说明 |
|------|------|
| 角色基础类 | ABaseCharacter，血量/法力/速度参数 |
| 玩家角色 | APlayerCharacter，FacingComponent、SkillSystemComponent |
| 敌人类 | AEnemyCharacter，基础移动参数 |
| 属性组件 | UAttributeComponent，血量/法力/攻击/防御属性 |
| 伤害计算组件 | UDamageCalculatorComponent，计算最终伤害（含暴击） |
| 朝向组件 | UFacingComponent，行走/注视模式切换 |
| 技能基类 | USkillBase，所有技能参数蓝图可配置，新增 GetInterruptibleAt() 虚函数 |
| 扇形斩击技能 | UMeleeSlashSkill，扇形范围伤害（延迟触发） |
| 跳跃技能 | UJumpSkill，抛物线位移跳跃，两阶段设计，GetInterruptibleAt() 强制飞行不可打断 |
| 技能系统组件 | USkillSystemComponent，技能队列/连招窗口/打断/延迟伤害/空指针过滤 |
| 右键全功能 | 统一 ActivateNextSkill() 流程，点击检测失败不阻断 |
| 自动攻击 | Tick 检测 bPendingAttack，到达距离后自动 ActivateNextSkill |
| 技能打断 | GetInterruptibleAt() 虚函数控制，子类可重写 |
| 延迟伤害 | DamageAt 时间点触发 ApplyDamage |
| 玩家控制器 | AWorldPlayerController，右键全功能操作 |
| 摄像机控制器 | UCameraControllerComponent，双轴独立弹性跟随 |

### 待开发（⬜）

| 优先级 | 模块 | 预估工时 |
|--------|------|---------|
| P1 | 直线穿刺技能 ULinearThrustSkill | 1-2 天 |
| P1 | 圆形范围技能 UCircleAoeSkill | 1-2 天 |
| P1 | 飞行投射物技能 UProjectileSkill | 2-3 天 |
| P1 | 敌人 AI 感知 + 行为树 | 3-4 天 |
| P2 | 装备数据结构 + 背包 UI | 5-7 天 |
| P2 | 商店系统 | 2-3 天 |
| P2 | 任务系统框架 | 5-7 天 |
| P3 | 阵形系统 | 5-7 天 |
| P3 | 好感度系统 | 3-4 天 |
| P3 | 打造/宝石系统 | 4-5 天 |

---

## 八、技术架构说明

### 8.1 项目结构

```
Source/Moliser3sGameClient/
├── BaseCharacter.h/.cpp           # 基础角色类
├── PlayerCharacter.h/.cpp         # 玩家角色
├── EnemyCharacter.h/.cpp          # 敌人角色
├── WorldPlayerController.h/.cpp   # 玩家控制器（右键全功能）
├── Component/
│   ├── Attribute/                 # 属性组件
│   ├── Damage/                    # 伤害计算组件
│   ├── Facing/                    # 朝向控制组件
│   ├── Input/                     # 点击检测组件
│   ├── Camera/                    # 摄像机控制器组件
│   └── Skill/                     # 技能系统组件
└── Skill/
    ├── SkillBase.h/.cpp           # 技能基类 ✅（含 GetInterruptibleAt 虚函数）
    ├── MeleeSlashSkill.h/.cpp     # 扇形斩击 ✅
    ├── JumpSkill.h/.cpp           # 跳跃技能 ✅
    ├── LinearThrustSkill/         # ⬜ 直线穿刺
    ├── CircleAoeSkill/            # ⬜ 圆形范围
    └── ProjectileSkill/           # ⬜ 飞行投射物
```

---

## 九、开发路线图

```
第一阶段：基础完善（已完成 ✅）
  ├─ [✅] 修复左键多技能 Bug
  ├─ [✅] 技能循环队列 + Duration 控制
  ├─ [✅] 蒙太奇动画播放
  ├─ [✅] 跳跃技能（抛物线位移）
  ├─ [✅] 连招窗口机制
  ├─ [✅] 技能打断（GetInterruptibleAt 虚函数）
  ├─ [✅] 延迟伤害（DamageAt）
  ├─ [✅] 右键全功能（统一 ActivateNextSkill）
  ├─ [✅] 自动攻击（bPendingAttack）
  ├─ [✅] 空指针防御（SkillList/Queue 过滤）
  └─ [✅] 摄像机控制器（双轴弹性跟随）

第二阶段：技能扩展（当前阶段）
  ├─ [⬜] 直线穿刺技能
  ├─ [⬜] 圆形范围技能
  ├─ [⬜] 飞行投射物技能
  └─ [⬜] 敌人共用技能系统

第三阶段：RPG 系统 ｜ 第四阶段：特色系统