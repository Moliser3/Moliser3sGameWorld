# 类暗黑破坏神 ARPG 游戏策划案

> 引擎：Unreal Engine 5.7  
> 语言：C++  
> 项目名称：Moliser3sGameClient  
> 策划日期：2026/06/12
> 最后更新：2026/06/14 22:45

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

### 2.3 跳跃（✅ 已完成）

- 跳跃作为技能类型（UJumpSkill）融入技能循环队列
- 右键点击地面设置目标位置，跳跃到目标（抛物线位移）
- 目标不可达时原地起跳
- 飞行中撞到障碍物则落地

**三段式生命周期（前摇→技能触发→后摇）：**
  - **前摇（WindupTime=0.72s）**：抛物线位移，**不可打断**
  - **技能触发（OnExecute）**：落地瞬间
  - **后摇（RecoveryTime=1.35s）**：落地收尾动画，**可被玩家打断**
  - **衔接时间（CustomLinkTime=0.2s）**：超时后技能组索引重置到第一个

**参数：** JumpRange=500, JumpHeight=200, WindupTime=0.72, RecoveryTime=1.35, CustomLinkTime=0.2

---

## 三、技能系统

### 3.1 技能组循环

玩家通过**鼠标右键**释放技能。每个角色拥有一个技能组（`SkillGroup`，蓝图配置），组内技能循环释放。

```
技能组（SkillGroup）:
┌──────┬──────┬──────┬──────┐
│ 技能A │ 技能B │ 跳跃 │ 技能C │
└──┬───┴──┬───┴──┬───┴──┬───┘
   │      │      │      │
   └──▶ 循环释放，到末尾回到开头
```

**索引规则：**
- 1 技能组：索引永远指向自己（可自打断自循环）
- 多技能组 + 衔接时间内点击：推进到下一技能
- 多技能组 + 衔接超时后点击：重置到第一个技能

### 3.2 打断与衔接机制（✅ 已完成）

```
Idle → ActivateNextSkill() → Windup（前摇，不可打断）
  │ WindupTime 到
  ▼
OnExecute（技能触发，瞬间）
  │
  ▼
Recovery（后摇，可打断）
  ├─ 玩家点击 → OnInterrupt() → 下一技能 Windup
  └─ RecoveryTime 到 → LinkWindow

LinkWindow（衔接时间）
  ├─ 玩家点击 → 下一技能 Windup（索引推进）
  └─ CustomLinkTime 超时 → GroupSkillIndex=0 → Idle
```

**衔接时间 = RecoveryTime + CustomLinkTime**，在此时间内点击均可推进索引。

### 3.3 技能类型

#### 类型一：扇形斩击（UMeleeSlashSkill ✅）
#### 类型二：跳跃（UJumpSkill ✅）

### 3.4 技能配置

所有参数暴露给蓝图，通过 DisplayName 显示中文名。

| 属性 | DisplayName | 说明 |
|------|------------|------|
| SkillName | — | 技能名称 |
| WindupTime | **前摇** | 技能起手时间（不可打断） |
| RecoveryTime | **后摇** | 收尾时间（可打断） |
| CustomLinkTime | **衔接时间** | 后摇后额外等待 |
| MaxSkillRange | **最大释放技能距离** | -1=无限制 |
| SkillCategory | — | 技能分类（Attack/Movement/Utility/Hybrid） |
| SkillMontage | — | 蒙太奇动画 |
| MontageSlotName | — | 槽位名称 |

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
| 技能基类 | USkillBase，前摇(Windup)/技能触发(OnExecute)/后摇(Recovery)/衔接(CustomLink) 三段式 |
| 伤害技能中间类 | UDamageSkillBase，OnExecute 时触发 ApplyDamage |
| 扇形斩击技能 | UMeleeSlashSkill，扇形范围伤害，激发瞬间触发 |
| 跳跃技能 | UJumpSkill，抛物线位移跳跃，前摇=飞行，后摇=落地收尾 |
| 技能系统组件 | USkillSystemComponent，技能组管理 + 四阶段状态机 |
| 右键全功能 | 根据 GetNextSkillCategory() 分类处理 |
| 自动攻击 | Tick 检测 bPendingAttack，到达距离后自动 ActivateNextSkill |
| 技能打断 | 后摇阶段全程可打断 |
| 技能组循环 | 1 技能自循环 / 多技能推进 + 衔接超时重置 |
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
    ├── SkillTypes.h               # 枚举定义 ✅
    ├── SkillBase.h/.cpp           # 技能基类 ✅（前摇/后摇/衔接时间）
    ├── DamageSkillBase.h/.cpp     # 伤害技能中间类 ✅
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
  ├─ [✅] 技能三段式生命周期（前摇/技能触发/后摇/衔接）
  ├─ [✅] 技能组系统（1 技能自循环 / 多技能推进）
  ├─ [✅] 蒙太奇动画播放
  ├─ [✅] 跳跃技能（抛物线位移）
  ├─ [✅] 技能打断（后摇可打断）
  ├─ [✅] 右键全功能（根据 SkillCategory 分类处理）
  ├─ [✅] 自动攻击（bPendingAttack）
  ├─ [✅] 技能分类枚举（Attack/Movement/Utility/Hybrid）
  └─ [✅] 摄像机控制器（双轴弹性跟随）

第二阶段：技能扩展（当前阶段）
  ├─ [⬜] 直线穿刺技能
  ├─ [⬜] 圆形范围技能
  ├─ [⬜] 飞行投射物技能
  └─ [⬜] 敌人共用技能系统

第三阶段：RPG 系统 ｜ 第四阶段：特色系统