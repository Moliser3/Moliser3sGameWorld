# 当前工作状态

> 最后更新：2026/06/12 14:54

## 当前阶段
第一阶段（基础完善）**全部完成** ✅

## 本日完成的工作（06/12 上午）

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

### 四、跳跃技能
- 抛物线位移（读取 `LastClickTarget`）
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

## 本日完成的工作（06/12 下午）

### 七、移动技能分类改造
- `SkillBase` 新增 `bIsMovementSkill` 属性，将跳跃技能标记为移动技能
- `SkillSystemComponent` 新增 `PeekNextSkill()` / `IsNextSkillMovement()` 接口
- `WorldPlayerController::OnRightMouseClick()` 流程重构：
  - 先检测下一个技能是否为移动技能 → 是则直接执行，不受攻击距离/敌人判断约束
  - 非移动技能走原有攻击/移动逻辑
- 效果：点击地面或敌人都能触发跳跃，跳跃不会再被"移动到攻击距离"流程拦截

### 八、文档目录重命名
- 目录 `策划案` → `Document`
- 所有文档文件名翻译为英文：
  - `小C角色信息.md` → `XiaoC_Role.md`
  - `当前工作状态.md` → `Current_Status.md`
  - `技术文档.md` → `Technical_Document.md`
  - `新对话启动流程.md` → `New_Conversation_Startup.md`
  - `类暗黑破坏神游戏策划案.md` → `DiabloLike_GameDesign.md`
  - `项目常识.md` → `Project_Knowledge.md`
- 更新文档内部相互引用路径
- 解决编译时中文路径导致的 UBT GitSourceFileWorkingSet 崩溃

## 下一阶段（待定）
- 🗡️ 直线穿刺技能
- 🔵 圆形范围技能
- 🏹 飞行投射物技能
- 🤖 敌人 AI + 行为树

## 当前已知问题
1. SimpleMoveToLocation 容差约 50cm，bPendingAttack 用 80cm 补偿
2. 跳跃抛物线的 Timer 驱动可能轻微抖动

## 对话备注
- **Moliser3**：项目拥有者
- **小C**：公司电脑上的 CLINE（XiaoC_Role.md）
- **大C**：家里电脑上的 CLINE（由 Moliser3 自行创建）
- **项目常识**：小C和大C共享（Project_Knowledge.md）