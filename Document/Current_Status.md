# 当前工作状态

> 最后更新：2026/06/15 14:00

## 当前阶段
第一阶段（基础完善）**全部完成** ✅
第二阶段（技能扩展）—— 打断机制统一化 + 自动攻击修复完成 ✅
第三阶段（玩家行为状态重构）—— 默认状态/战斗状态已完成 ✅

## 已完成工作（06/15）

### 二十七、玩家行为状态重构（默认状态/战斗状态）
- **新增 `EPlayerState` 枚举**：`Default(默认状态)` / `Battle(战斗状态)`，在 `WorldPlayerController` 中管理
- **状态转移**：点击敌人 → Default → Battle；敌人超出战斗感知范围或无目标 → Battle → Default
- **默认状态**：点击地面行走(300)，Shift+点击奔跑(600)；点击敌人锁定并移向敌人
- **战斗状态右键敌人**：循环释放技能（复用原有逻辑）
- **战斗状态右键地面**：默认行走(300)面朝敌人；Shift奔跑(600)面朝移动方向，到达后恢复面朝敌人
- **位移技能恢复注视**：战斗状态下释放位移技能后，技能结束自动恢复注视模式面朝敌人
- **Shift键检测**：`IsInputKeyDown(EKeys::LeftShift)`，直接在 C++ 中判断无需蓝图额外绑定
- **新增 `BattlePerceptionRange`（战斗感知范围）**：`PlayerCharacter` 蓝图可配置，默认 1500
- **FacingComponent 简化**：移除距离自动清除注视逻辑（由 Controller 统一管理）

## 已完成工作（06/14）

### 二十三、技能系统全面重构（前摇/技能触发/后摇/衔接时间）
- **技能阶段重构**：原 `Duration` + `DamageAt` + `InterruptibleAt` 线性模型 → `WindupTime` + `RecoveryTime` + `CustomLinkTime` 三段式模型
- **技能基类** (`SkillBase`)：移除 `Duration`、`InterruptibleAt`、`ComboWindowDuration`、`Update()`、`GetInterruptibleAt()`；新增 `WindupTime`、`RecoveryTime`、`CustomLinkTime`、`OnWindupUpdate()`、`OnExecute()`、`OnRecoveryUpdate()`
- **伤害技能基类** (`UDamageSkillBase`)：移除 `DamageAt`，伤害由 `OnExecute()` 在激发瞬间触发
- **近战技能** (`MeleeSlashSkill`)：`OnExecute()` 直接调用 `ApplyDamage()`，移除 `Radius` 属性，统一使用 `MaxSkillRange`
- **跳跃技能** (`JumpSkill`)：前摇=抛物线飞行(0.72s)，后摇=落地收尾(1.35s)，移除 `EndJump()` 和 `ForceEndCurrentSkill()` 依赖
- **技能组系统**：`SkillList` / `SkillQueue` 合并为单一 `SkillGroup`，支持 1 技能自循环

### 二十四、技能组索引与衔接时间
- **衔接时间** = 后摇时间 + 自定义衔接时间
- **衔接时间内点击** → 推进到组内下一个技能
- **衔接超时后点击** → 重置索引到第一个技能
- **1 技能组特例**：索引永远指向自身，连续点击可自打断自循环

### 二十五、命名规范补充
- `bIsMovementSkill` → `ESkillCategory` 枚举（Attack/Movement/Utility/Hybrid）
- `MaxAttackRange` → `MaxSkillRange`（蓝图 DisplayName = "最大释放技能距离"）
- `IsNextSkillMovement()` → `GetNextSkillCategory()`
- `GetMaxAttackRange()` → `GetMaxSkillRange()`
- 项目中新增命名规范：代码标识符用英文，提示/注释/DisplayName 用中文

### 二十六、代码结构清理
- `PlayerCharacter::BeginPlay()` 移除默认技能注册（技能完全由蓝图配置）
- `SkillSystemComponent` 移除已废弃接口：`IsSkillActive()`、`IsInComboWindow()`、`GetQueueIndex()`、`TryInterruptCurrentSkill()`、`ForceEndCurrentSkill()`、`GetCurrentSkillElapsed()`
- `TickComponent` 状态机重写为 `Idle → Windup → Recovery → LinkWindow`

## 当前已确认正常工作
1. 跳跃技能 → 前摇抛物线飞行 → 技能触发落地 → 后摇收尾 ✅
2. 后摇期右键点击 → 打断当前技能 → 组内下一技能前摇 ✅
3. 1 技能组（仅跳跃）→ 后摇期打断 → 重新跳跃 ✅
4. 衔接时间内点击 → 推进到下一技能 ✅
5. 衔接超时后点击 → 重置索引到第一个技能 ✅
6. 近战技能右键点击敌人 → 距离检查 + 自动攻击 ✅
7. 右键点击地面 → 位移技能直接执行 / 非位移技能移动 ✅
8. 默认状态点击地面行走(300) / Shift奔跑(600) ❓待测试
9. 点击敌人进入战斗状态 + 移向敌人攻击 ❓待测试
10. 战斗状态右键敌人循环释放技能 ❓待测试
11. 战斗状态右键地面面朝敌人行走 ❓待测试
12. 战斗状态Shift奔跑面朝移动方向 + 到达后恢复面朝敌人 ❓待测试
13. 位移技能结束后恢复注视模式面朝敌人 ❓待测试
14. 超出战斗感知范围自动退出战斗状态 ❓待测试

## 已知问题
- SimpleMoveToLocation 的精确度有限（约 50cm 容差），Tick 中 bPendingAttack 的到达判断使用 80cm 容差补偿

## 对话备注
- **Moliser3**：项目拥有者
- **小C**：AI游戏开发助手 （XiaoC_Role.md）

