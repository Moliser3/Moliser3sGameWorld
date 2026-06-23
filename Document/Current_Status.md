# 当前工作状态

> 最后更新：2026/06/23 17:55

## 当前阶段
第一阶段（基础完善）**全部完成** ✅
第二阶段（技能扩展 / 需求变更）**全部完成** ✅
第三阶段（正交状态机重构）—— 双轴状态 + 事件驱动已完成 ✅
**第四阶段（数据先行 — P2）基本完成** ✅  
**第五阶段（背包系统 — P3）搭建中 🚧**

## 已完成工作（06/22）

### 三十九、Debug 装备测试（13槽位直接赋值）
- **`BaseCharacter.h`**：新增 13 个 `UPROPERTY(Instanced)` 装备槽属性（头盔~副手），蓝图中直接创建 `UEquipItem` 实例
- **`BaseCharacter.cpp`**：`BeginPlay` 中自动调用 `EquipmentComponent->EquipItem()`
- **标记删除**：所有 Debug 代码以 `【Debug 装备测试 — 上线前需删除】` 标记

### 四十、伤害日志中文化 + 装备加成显示
- **`MeleeSlashSkill.cpp`**：所有 UE_LOG 和屏幕 Debug 信息改为中文
- **增加输出**：攻击方/防御方的装备五行加成（`装备加成[金+%d 木+%d ...]`）

### 四十一、防御系统改为百分比减免
- **`DamageCalculatorComponent.cpp`**：防御从固定减法改为 `Defense / (Defense + 100)` 百分比减免
- 公式：`减免率 = 防御值 / (防御值 + 100)`
- 解决外伤占比不同但总伤害相同的问题
- **日志相应更新**：显示外防/内防的数值、减免百分比、减免量

### 四十二、UItemBase 扩展 + 消耗品体系
- **`ItemBase.h`**：新增 `WorldMesh`（地面静态网格体）、`Use()` 虚函数、`GetIcon()`/`GetWorldMesh()` 缓存加载方法
- **新增 `Data/ConsumableItem.h/.cpp`**：`UConsumableItem`，含 `EConsumableEffectType`（HealHP/RestoreMP/Buff）
- `Use()` 实现：根据 EffectType 调用 `AttributeComponent->Heal()` 或 `RestoreMana()`

### 四十三、背包系统（InventoryComponent）
- **新增 `Component/Inventory/InventoryComponent.h/.cpp`**：
  - `TArray<UItemBase*> Items`，默认 30 格
  - `AddItem(自动堆叠)` / `RemoveItem` / `DropItem(丢在地上)` / `UseItem`
  - `OnInventoryChanged` 事件广播（UMG UI 刷新用）
- **挂在 `ABaseCharacter`** 上，所有角色自带背包

### 四十四、快捷栏系统（QuickSlotComponent）
- **新增 `Component/Inventory/QuickSlotComponent.h/.cpp`**：
  - 8 格快捷栏，`AssignSlot` / `ClearSlot` / `UseSlot`
  - `OnQuickSlotChanged` 事件广播
- **挂在 `APlayerCharacter`** 上，仅玩家拥有
- 数字键 1-8 在蓝图中绑定 `UseSlot()`

### 四十五、地面物品 Actor
- **新增 `WorldActors/WorldItemActor.h/.cpp`**：
  - `UStaticMeshComponent`（显示 WorldMesh）
  - `UWidgetComponent`（显示 Icon，预留）
  - `OnPickup()` → 调用拾取者的 `InventoryComponent->AddItem()` 后销毁
  - `InitializeFromItem()` 从物品数据设置可视表现

### 四十六、装备组件清理
- **删除**消耗品槽位（`EConsumableSlot` / `EquipConsumable` / `AutoConsume`）—— 设计简化，NPC 直接扫描背包使用消耗品
- **`EConsumableEffectType`** 从 `EquipmentData.h` 移至 `ConsumableItem.h`

### 四十七、Debug 背包测试
- **`BaseCharacter.h`**：新增 `TestInventoryItems` 数组，蓝图中直接赋值，`BeginPlay` 自动入背包
- **标记删除**：以 `【Debug 背包测试 — 上线前需删除】` 标记

### 四十八、技能范围改为阶段级配置
- **`FSkillStage` 新增 `SkillRange`**（DisplayName="技能范围"），替代原有的技能级 `MaxSkillRange`
- **`SkillBase` 移除 `MaxSkillRange`**，新增 `GetSkillRange()` 从当前阶段读取
- **`MeleeSlashSkill::ApplyDamage`** 改使用 `GetSkillRange()` 进行球形检测
- **`GetMaxSkillRange()`** 改为遍历所有技能的所有阶段取最大值
- **`JumpSkill` 适配**：`Stage0.SkillRange = -1`
- **修复问题**：第三阶段前摇较长时角色偏移导致 `MaxSkillRange` 不够大而无法命中的 Bug

## 已完成工作（06/23）

### 四十九、UI点击穿透修复（UMG遮挡鼠标射线）
- **`WorldPlayerController` 新增 `IsMouseOverUI()`**：使用 `FSlateApplication::Get().GetCursorPos()` + `LocateWindowUnderMouse` 检测鼠标下是否有 `SObjectWidget`（UMG内容控件）
- **DPI兼容**：使用 `GetCursorPos()` 替代 `GetMousePosition()`，解决窗口模式右下角坐标偏移问题
- **`OnLeftMouseClick` / `OnRightMouseClick`** 开头调用 `IsMouseOverUI()` 提前返回
- **开启 `Slate` + `SlateCore` 模块依赖**
- **正确行为**：有内容的UI（背包/按钮/Border）阻挡射线，空Canvas Panel不阻挡

### 五十、背包拖拽交换系统
- **`InventoryComponent` 新增 `SwapItems(IndexA, IndexB)`**：交换两个格子的物品，自动填充 `nullptr` 到未初始化的空格
- **`SwapItems` 支持拖到空格**：目标索引超出数组时自动 `Add(nullptr)` 填充
- **`BeginBatch()` / `EndBatch()`**：批量操作时不触发 `OnInventoryChanged`，`EndBatch()` 时统一触发一次
- **`BaseCharacter::BeginPlay` 适配**：测试物品导入用 `BeginBatch/EndBatch` 包裹
- **`BroadcastChange()` 内部辅助方法**：替代直接 `OnInventoryChanged.Broadcast()`，批量模式下静默

### 五十一、拖拽Debug日志（随后清理）
- 添加 `[Swap]` 调试日志验证交换逻辑正确性（已清理）
- 添加 `[UI调试]` 日志排查点击穿透问题（已清理）

## 已完成工作（06/21）

### 三十二、角色五维数据结构定义
- **新增 `Data/CharacterData.h`**：`FCharacterCoreData` 结构体，包含：
  - 五行根基值（金/木/水/火/土），可在蓝图编辑
  - 五维属性计算（劲力=金+土×30%，气血=木+水×30%，内息=水+金×30%，身法=火+木×30%，体魄=土+火×30%）
  - 派生战斗属性（攻击力/最大血量/最大法力/生命恢复/法力恢复/防御/移速加成/闪避率/暴击率）含上限
- **新增 `Data/DataDefinitions.h`**：`EWuXing` 五行枚举
- **新增 `Data/` 目录**：后续装备/宝石/锻造数据均位于此
- **改造 `AttributeComponent`**：移除硬编码的 `MaxHealth/MaxMana/BaseDamage/CritRate/Armor`，全部改为从 `FCharacterCoreData` 派生
- **保留接口兼容**：`GetBaseDamage()`、`GetCritRate()`、`GetCritMultiplier()`、`GetArmor()`、`GetDamageReduction()` 接口不变，内部逻辑已切换

### 三十三、装备栏槽位定义
- **新增 `Data/EquipmentData.h`**：
  - `EEquipmentSlot` 枚举（14个槽位：头盔/肩甲/胸甲/护腕/手套/腰带/裤子/靴子/项链/戒指×2/主手武器/副手）
  - `EWeaponUsage` 枚举（单手/双手），主手单手可配副手，双手占用并锁定副手
  - `EItemRarity` 枚举（普通/魔法/稀有/独特）
  - `FEquipmentItemData` 结构体（物品ID/名称/描述/槽位/稀有度/武器类型/等级需求）

### 三十四、物品类体系 + 装备实装
- **新增 `Data/ItemBase.h`**：`UItemBase` 物品基类（UObject，支持蓝图派生），包含ID/名称/描述/图标/堆叠上限
- **新增 `Data/EquipItem.h/.cpp`**：`UEquipItem` 可装备物品，继承 `UItemBase`，包含槽位/武器类型/等级需求/五行加成（金木水火土各一个Bonus字段）
- **新增 `Component/Equipment/EquipmentComponent.h/.cpp`**：装备管理组件，管理14槽位 TMap，支持 EquipItem/UnequipItem，自动处理双手武器锁定副手逻辑，装备/卸下时自动加减五行加成到 CharacterData
- **改造 `CharacterData.h`**：新增 `AddEquipmentBonus()` / `RemoveEquipmentBonus()` 接口
- **改造 `BaseCharacter.h/.cpp`**：新增 `EquipmentComponent` 组件 + `GetEquipmentComponent()` 便捷接口

### 三十五、伤害计算重构（五行生克 + 外伤/内伤）
- **新增 `ESkillWuXing` 枚举**（金木水火土），用于技能配置五行属性
- **新增 `SkillBase` 字段**：`SkillWuXing`（技能五行）+ `ExternalDamageRatio`（外伤占比）
- **改造 `CharacterData.h`**：`GetDefense()` 拆分为 `GetExternalDefense()`（体魄+劲力）和 `GetInternalDefense()`（体魄+内息）；新增 `GetDominantWuXing()`（取最高五行作为目标属性）
- **改造 `FDamageResult`**：新增 `WuXingMultiplier`、`ExternalDamage`、`InternalDamage`、`ExternalDefenseReduced`、`InternalDefenseReduced`
- **重写 `CalculateDamage`**：完整链路为 基础伤害 → 五行生克(±30%) → 暴击 → 外伤/内伤拆分 → 外防/内防减免 → 百分比减伤 → 最终伤害
- **五行相克查表**：静态函数 `GetWuXingMultiplier()`，使用 5×5 查表实现，攻方克守方×1.3，守方克攻方×0.7
- **适配 `MeleeSlashSkill`**：`CalculateDamage` 调用传入 `SkillWuXing` 和 `ExternalDamageRatio`
- **移除旧接口**：`GetArmor()`、`GetDefense()` 已替换为 `GetExternalDefense()`/`GetInternalDefense()`

### 三十六、技能五行属性移入技能阶段
- **`FSkillStage` 新增字段**：`SkillWuXing`（阶段五行） + `ExternalDamageRatio`（外伤占比），每阶段可独立配置
- **`SkillBase` 移除字段**：`SkillWuXing`、`ExternalDamageRatio` 移到 `Stage` 中
- **适配 `MeleeSlashSkill`**：读取 `Stage.SkillWuXing`、`Stage.ExternalDamageRatio`

### 三十七、调试日志系统
- **基础属性日志**：`ABaseCharacter::BeginPlay()` 中使用 `UE_LOG` 输出角色名/五行/五维/派生属性，所有角色出生时自动打印
- **伤害分解日志**：`MeleeSlashSkill::ApplyDamage()` 中每条伤害信息同时通过 `UE_LOG`（终端）和 `AddOnScreenDebugMessage`（屏幕）输出

### 三十八、HP/MP 初始化修复
- **修复**：`AttributeComponent::BeginPlay()` 初始化 `Health/GetMaxHealth()`、`Mana=GetMaxMana()`，解决重构后角色出生血量为0的问题

## 已完成工作（06/18）

### 三十一、正交状态机重构（双轴状态 + 事件驱动）
- **新增 `GamePlayerState.h`**：定义 `ECombatState(Default/BattlePerception)` + `EActionState(Idle/Walking/Running/Skill)` 双轴
- **替换单轴 `EPlayerState`**：从 Controller 移除，拆分为双轴独立管理
- **事件驱动**：`OnCombatStateChanged` / `OnActionStateChanged` 广播委托
- **PlayerCharacter 监听回调**：根据(Combat, Action)组合矩阵决定面朝+速度
- **组合行为矩阵**：
  | Combat | Action | 面朝 | 速度 |
  |--------|--------|------|------|
  | Default | Walking | 移动方向 | 300 |
  | Default | Running | 移动方向 | 600 |
  | Battle | Walking | 目标 | 300 |
  | Battle | Running | 移动方向 | 600 → 到达Idle后恢复目标 |
  | * | Skill | 点击方向 | 0(停移动) |
- **FacingComponent 独立**：仅管理 Aiming/Walking 模式切换，不关心来源
- **清理**：移除 `bPendingRestoreAiming`、`bPreviousSkillActive`、`EPlayerState`

## 已完成工作（06/15）

### 二十七、玩家行为状态重构（已废弃 → 被 06/18 三十一替代）

### 二十八、多阶段技能系统 + 双技能组
- **新增 `ESkillType` 枚举**：标识技能类型（直拳/勾拳等），用于阶段推进判断
- **新增 `FSkillStage` 结构体**：每个阶段独立配置前摇/后摇/衔接时间/蒙太奇/基础伤害/扇形半角/最大高度差
- **`SkillBase` 改造**：删除单阶段属性 `WindupTime/RecoveryTime/CustomLinkTime/SkillMontage/MontageSlotName`，替换为 `TArray<FSkillStage> Stages` 数组 + `ESkillType SkillType`
- **阶段推进规则**：连续出现同类型技能 → `StageIndexForType++`；不同类型 → `StageIndexForType = 0`
- **`MeleeSlashSkill` 简化**：删除 `HalfAngleDeg/BaseDamage/MaxZDiff`，`ApplyDamage()` 改为从 `Stages[CurrentStage]` 读取
- **`JumpSkill` 适配**：`WindupTime=0.72/RecoveryTime=1.35/CustomLinkTime=0.2` 移入 `Stages[0]`
- **`PlaySkillMontage` 修复**：`Montage_Stop(0.2f)` 去掉参数，停止所有蒙太奇而非仅停止指定蒙太奇

### 二十九、双技能组系统
- **`SkillSystemComponent`**：单 `SkillGroup` → 拆分为 `LeftSkillGroup` + `RightSkillGroup`
- **左右独立索引**：`LeftGroupIndex` / `RightGroupIndex`，各自独立推进
- **左右独立阶段追踪**：`LeftLastSkillType` / `LeftStageForType` / `RightLastSkillType` / `RightStageForType`
- **`ActivateLeft()`**：左键技能激活（用于点击敌人的距离检查+攻击）
- **`ActivateRight()`**：右键技能激活（无距离检查，直接释放）
- **衔接超时**：重置左右组阶段追踪归零

### 三十、鼠标左键恢复 + 右键简化 + Alt防御 + 变量清理
- **`OnLeftMouseClick()`**：恢复实现，点击地面移动，点击敌人锁定+距离检查+攻击
- **`OnRightMouseClick()`**：简化，点击敌人/地面直接释放右键技能，无距离检查
- **`OnAltPressed/Released`**：新增 Alt 防御状态 `bDefending`
- **清理**：移除 `LockedWalkSpeed(150)` / `LockedRunSpeed(300)` / `LockOnRange(1000)` / `bAimingFullSpeed` / `SetAimingFullSpeed()`

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

