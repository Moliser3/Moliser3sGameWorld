# 当前工作状态

> 最后更新：2026/06/12 20:12

## 当前阶段
第一阶段（基础完善）**全部完成** ✅

## 今日完成工作（06/12）

### 一、右键全功能改造
- 废弃左键，所有操作通过鼠标右键完成
- 右键点击敌人：近距直接攻击，远距走到攻击距离后自动攻击
- 右键点击地面：移动
- `WorldPlayerController` 新增 `LastClickTarget`、`bPendingAttack`、`PendingMaxRange`

### 二、自动攻击机制
- Tick 中检测 `bPendingAttack` 标记
- 到达 `MaxAttackRange` 或速度接近零时自动触发 `ActivateNextSkill()`
- 跳过 `IsSkillActive()` 拦截，让打断逻辑能正常进入

### 三、技能打断修复
- `InterruptibleAt` 默认值改为 `0.3`（蓝图可配置）
- 移除 `OnRightMouseClick` 和 `Tick` 中的 `IsSkillActive()` 拦截
- 打断逻辑现在能正常执行

### 四、跳跃技能（抛物线位移）
- 读取 `LastClickTarget` 作为目标位置
- NavMesh 检测可达性，不可达则原地起跳
- 碰撞检测落地
- 新增 `JumpRange`、`JumpHeight`、`FlyDuration` 参数

### 五、代码清理
- `MinAttackRange` → `MaxAttackRange`
- `GetNextAttackRange()` → `GetMaxAttackRange()`
- `StopDistance` 完全移除
- `InterruptibleAt` 默认值 0.3

### 六、文档体系完善
- `Technical_Document.md` 和 `DiabloLike_GameDesign.md` 同步更新
- 新增 `Current_Status.md` — 项目进度快照
- 新增 `Project_Knowledge.md` — 小C和大C共享的项目知识
- 新增 `XiaoC_Role.md` — 身份和工作风格
- 目录 `策划案` → `Document`，文件名翻译为英文
- 解决编译时中文路径导致的 UBT GitSourceFileWorkingSet 崩溃

### 七、移动技能分类
- `SkillBase` 新增 `bIsMovementSkill` 属性，跳跃标记为移动技能
- `SkillSystemComponent` 新增 `PeekNextSkill()` / `IsNextSkillMovement()` 接口
- `WorldPlayerController::OnRightMouseClick()` 流程重构：
  - 移动技能不受攻击距离/敌人判断约束，点击即触发
  - 非移动技能走原有攻击/移动逻辑
- 效果：点击地面或敌人都能触发跳跃

### 八、跳跃飞行阶段 — 输入拦截
- 修复 Bug：跳跃快要落地时连续点击鼠标，落地后变成奔跑
- `WorldPlayerController::OnRightMouseClick()` 入口拦截：
  - 移动技能 + elapsed < InterruptibleAt → 忽略所有输入
  - 收尾阶段（elapsed ≥ InterruptibleAt）→ 放行
- `SkillSystemComponent` 新增 `GetCurrentSkillElapsed()` 接口

### 九、SkillBase 虚函数体系扩展
- 新增 `virtual void Update(Instigator, DeltaTime)` — 每帧更新
- 新增 `virtual void OnInterrupt(Instigator)` — 被打破时清理状态
- `SkillSystemComponent::TickComponent` 中调用 `CurrentSkill->Update()`

### 十、两阶段跳跃技能
- **阶段1（0 ~ FlyDuration）**：抛物线位移，Update 驱动
- **阶段2（FlyDuration ~ Duration）**：落地收尾，角色不再移动
- `InterruptibleAt = FlyDuration`：飞行中不可打断，收尾期可打断
- 跳跃默认参数：`Duration=2.07`, `FlyDuration=0.72`, `InterruptibleAt=0.72`
- 移除跳跃中的 Timer 驱动，改为 SkillSystemComponent 的 Tick 驱动（消除卡顿）
- 移除 `IsFalling()` 检查，仅用 `bIsJumping` 判断（消除二次跳跃被拦截）

### 十一、打断机制完善
- `SkillSystemComponent` 新增 `TryInterruptCurrentSkill()` 接口
- `ActivateNextSkill` 打断后 `goto ExecuteSkill` 跳过 IDLE 重置 QueueIndex
- IDLE 状态不再重置 QueueIndex=0，保留当前位置
- 收尾阶段点击地面 → 先打断再移动
- 飞行阶段拦截不挡收尾期输入
- 关闭运动模糊（MotionBlurAmount=0）

### 十二、SpringArm → 自定义 CameraController
- `PlayerCharacter` 移除 `USpringArmComponent`
- `CameraComponent` 独立于角色坐标系（不 SetupAttachment）
- 新增 `UCameraControllerComponent`（挂载在 `WorldPlayerController`）
- 双轴独立弹性跟随：水平（X/Y）和垂直（Z）独立参数
- 跳跃期间通过 `SetJumping(true)` bypass Z 弹性，落地后恢复
- 相机轴不变：固定旋转 `(-60°, 0, 0)`
- 关掉蒙太奇播放时 `MontageSlotName.IsNone()` 的条件判断，全程先 stop 再 play

### 十三、SpringArm 恢复 → 再回自定义 CameraController
- 试过 SpringArm 的 `bEnableCameraLag`（引擎自带弹性），但因 Z 轴弹性不可独立控制 + 跳跃 `TeleportPhysics` 导致的抖动，最终回到自定义方案

### 十四、OnRightMouseClick 统一技能化
- 移除 `IsNextSkillMovement()` 特殊判断分支
- 所有点击统一走 `ActivateNextSkill()`，跳跃就是技能
- 飞行阶段拦截 + 收尾放行 + 打断机制由技能系统内部处理
- 目前仍存在 Bug：跳跃收尾后再次跳跃时，QueueIndex 指向错位，正在排查

## 下一阶段（待定）
- 🗡️ 直线穿刺技能
- 🔵 圆形范围技能
- 🏹 飞行投射物技能
- 🤖 敌人 AI + 行为树

## 当前已知问题
1. SimpleMoveToLocation 容差约 50cm，bPendingAttack 用 80cm 补偿
2. 跳跃收尾期打断后，再次跳跃时 QueueIndex 指向非移动技能导致行走（待解决）

## 对话备注
- **Moliser3**：项目拥有者
- **小C**：公司电脑上的 CLINE（XiaoC_Role.md）
- **大C**：家里电脑上的 CLINE（由 Moliser3 自行创建）
- **项目常识**：小C和大C共享（Project_Knowledge.md）