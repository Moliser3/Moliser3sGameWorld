# 当前工作状态

> 最后更新：2026/06/12 23:46

## 当前阶段
第一阶段（基础完善）**全部完成** ✅
第二阶段（技能扩展）—— 跳跃技能打断/收尾期 Bug 修复中

## 今日完成工作（06/12，继续）

### 十五、WorldPlayerController 右键点击流程重写
- 移除 `IsNextSkillMovement()` 特殊判断分支，所有点击统一走 `ActivateNextSkill()`
- 点击检测失败不再阻断技能执行：点击检测仅用于更新 `LastClickTarget` 和注视目标
- 飞行拦截改用 `GetInterruptibleAt()` 虚函数（解决蓝图覆盖 `InterruptibleAt` 导致飞行中被错误打断的问题）

### 十六、SkillSystemComponent 空指针防御
- `ActivateNextSkill()` auto-populate 时**遍历 SkillList 只加有效技能**（过滤空指针残留）
- `SetSkillQueue()` 调用时也**过滤空指针**后再填充队列
- `PeekNextSkill()` 循环跳过空指针，找到有效技能再返回
- `ActivateNextSkill()` 的 ExecuteSkill 阶段循环查找有效技能
- 新增 `IsInComboWindow()` 和 `GetQueueIndex()` 接口

### 十七、GetInterruptibleAt() 虚函数体系
- `SkillBase` 新增 `virtual float GetInterruptibleAt() const`，默认返回 `InterruptibleAt` 属性
- `JumpSkill` 覆盖 `GetInterruptibleAt()` 返回 `FMath::Max(InterruptibleAt, FlyDuration)`
- `SkillSystemComponent` 中 `ActivateNextSkill()` 和 `TryInterruptCurrentSkill()` 均使用 `GetInterruptibleAt()`
- `WorldPlayerController` 飞行拦截使用 `GetInterruptibleAt()`
- 屏幕调试倒计时使用 `GetInterruptibleAt()` 判断可打断时机
- **效果**：蓝图中错误覆盖 `InterruptibleAt=0.30` 不再影响飞行阶段的不可打断性

### 十八、调试辅助
- 屏幕显示跳跃飞行/后摇倒计时（红色=飞行中不可打断，黄色=后摇可打断）
- 屏幕显示点击检测命中/未命中
- 屏幕显示跳跃目标可达性
- 日志完善：`[JumpSkill] EXECUTE` 输出起点/目标/距离等信息

## 当前已知问题
1. SimpleMoveToLocation 容差约 50cm，bPendingAttack 用 80cm 补偿
2. `Duration`（3.50）和 `FlyDuration`（0.72）等参数被蓝图覆盖，与代码默认值不一致
3. 收尾期第一次点击有时不生效（`JumpTargetLoc == JumpStartLoc` 导致的原地起跳感觉不到位移 / `IsTargetReachable()` 短暂返回 false）

## 对话备注
- **Moliser3**：项目拥有者
- **小C**：公司电脑上的 CLINE（XiaoC_Role.md）
- **大C**：家里电脑上的 CLINE（DaC_Role.md）
- **项目常识**：小C和大C共享（Project_Knowledge.md）