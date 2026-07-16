# 当前工作状态

> 最后更新：2026/07/16 15:00

## 当前阶段
第一阶段（基础完善）**全部完成** ✅
第二阶段（技能扩展 / 需求变更）**全部完成** ✅
第三阶段（正交状态机重构）—— 双轴状态 + 事件驱动已完成 ✅
**第四阶段（数据先行 — P2）基本完成** ✅  
**第五阶段（背包系统 — P3）搭建中 🚧**  
**第六阶段（快捷栏系统 — 终极）搭建中 🚧**  
**第七阶段（路径表现组件）已完成** ✅

## 已完成工作（07/16）

### 六十九、路径表现组件（UPathDisplayComponent）
- **新增 `Component/PathDisplay/PathDisplayComponent.h/.cpp`**：
  - Catmull-Rom 样条插值生成平滑路径，而非直线连接
  - `StepLength`：步长控制采样间隔
  - `StepSubdivision`：步长边细分数量，使拐弯处 Perp 过渡平滑
  - `PathWidth`：路径宽度
  - `EndMarkerSize`：终点标记大小（独立 Section 1）
  - 连续 UV 映射：V = 累计路径距离 / StepLength，不归零
  - 交替三角形绕序：相邻 quad 交替对角线方向，消除固定折痕
  - 法线固定为 `(0,0,1)`，全部 CCW 绕序
  - 终点标记有独立材质 `EndMarkerMaterial`
- **公开接口**：
  - `SetNewPathMesh(PathNodes, StepLength, PathWidth, EndMarkerSize)` — 主入口，清空旧 Section 0/1 后重建
  - `SetEndMarkerMaterial` — 设置终点标记材质
  - `SetStepLength` / `SetPathWidth` / `SetStepSubdivision` / `SetEndMarkerSize` — 单独修改
- **迁移至 DVD 项目**：复制到 `D:\UEProject\DVD\Source\Skull`，适配 API 宏 `SKULL_API`

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

### 四十四、快捷栏系统（QuickSlotComponent）→ [06/23 晚更新]
- **`QuickSlotComponent`**：初始 8 格 → **10 格**（`SlotCount = 10`），新增 `SwapSlots(IndexA, IndexB)` 支持快捷栏内部交换
- **`UseSlot` 改造**：使用后自动 `ClearSlot(Index)`（物品消耗即消）
- **`WorldPlayerController` 新增 `OnQuickSlotKeyPressed(SlotIndex)`**：数字键 1-0 入口，取 `PlayerCharacter → GetQuickSlot → UseSlot`

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

## 已完成工作（06/24）

### 五十四、物品堆叠数量系统 + 拆分功能
- **`InventoryComponent`**：新增 `ItemCounts` 并行计数数组，`AddItem`/`RemoveItem`/`DropItem`/`UseItem`/`SwapItems` 全部适配数量管理
- **`QuickSlotComponent`**：新增 `ItemCounts` 并行计数数组，`AssignSlot`/`ClearSlot`/`UseSlot`/`SwapSlots`/`DropSlotItem` 全部适配数量管理
- **`SplitItem(SourceSlot, Count)`**：新增拆分函数，从源格拆出指定数量到空格，无空格则返回 false
- **`SetItemAt(Slot, Item, Count)`**：新增直接写入函数，用于背包↔快捷栏交换时指定目标格
- **`AddItem(Item, Count)`**：支持批量添加，自动堆叠+找空格
- **`GetCountAt(Slot)`**：新增获取每格数量的函数
- 涉及文件：`InventoryComponent.h/.cpp`，`QuickSlotComponent.h/.cpp`

### 五十五、物品拖拽叠加功能
- **`TryStackOrSwap(Source, Target)`**：同 ID 且未满时叠加数量，否则退化为交换
- 背包和快捷栏各有一套实现
- **`SwapWithInventory`** 新增方向感知：`bFromInventory` 参数指定拖拽来源，确保正确方向优先叠加
- 涉及文件：`InventoryComponent.h/.cpp`，`QuickSlotComponent.h/.cpp`

### 五十六、拖拽操作数据结构（C++）
- **新增 `UI/ItemDragDropOperation.h`**：`UItemDragDropOperation`（继承 `UDragDropOperation`）
- **`ESlotContainerType`**：`Inventory` / `QuickSlot` 枚举
- **`SourceContainer`**、`SourceSlotIndex`、`DraggedItem** 字段，蓝图中可直接读写
- 涉及文件：`Public/UI/ItemDragDropOperation.h`

### 五十七、背包↔快捷栏交互函数（组件化重构）
- **`QuickSlotComponent::SwapWithInventory(InvIdx, QSIdx, bFromInventory)`**：统一处理背包与快捷栏之间的交换/叠加
- **`QuickSlotComponent::DropSlotItem(Index)`**：丢弃快捷栏物品到场景，与 `Inventory.DropItem` 调用方式统一
- **`Controller::DropQuickSlotItem`** 改为委托给 `QuickSlotComponent::DropSlotItem`
- **移除** `WorldPlayerController::SwapInventoryWithQuickSlot`、`TransferInventoryToQuickSlot`、`TransferQuickSlotToInventory`
- 涉及文件：`QuickSlotComponent.h/.cpp`，`WorldPlayerController.h/.cpp`

### 五十八、Debug 快捷栏测试
- 沿用 `BaseCharacter::TestInventoryItems` 模式，`PlayerCharacter` 新增 `TestQuickSlotItems` 数组
- 蓝图中可直接赋值，`BeginPlay` 自动填入对应槽位

### 五十九、全链路 Debug 日志
- `[背包]` 标签：`AddItem`/`RemoveItem`/`DropItem`/`UseItem`/`SwapItems`/`TryStackOrSwap`/`SplitItem`
- `[快捷栏]` 标签：`AssignSlot`/`ClearSlot`/`UseSlot`/`SwapSlots`/`DropSlotItem`/`SwapWithInventory`/`TryStackOrSwap`/`SplitItem`
- `[Controller]` 标签：快捷键按下、`DropQuickSlotItem`
- 每个函数输出：操作名+槽位索引+物品ID+数量+结果

## 已完成工作（06/23 晚）

### 五十二、背包拖拽丢弃重构（纯蓝图交互 + 纯数据组件）
- **`InventoryComponent` 改为纯数据组件**：移除 `TickComponent`、`bDragActive`、`DragSourceSlotIndex`、`BeginDrag`/`EndDrag`
- **`RemoveItem` 索引稳定化**：`Items.RemoveAt()+Add(nullptr)` → `Items[SlotIndex] = nullptr`，拖拽交换/丢弃不再打乱其他物品位置
- **`DropItem` 完善**：丢弃时在角色前方 100cm 生成 `WorldItemActor`（带静态网格体），背包格子置空，可走过去拾取
- **拖拽检测完全由 UMG 蓝图驱动**（InventoryComponent 不开启 Tick）：
  - `WBP_InventorySlot.OnDragDetected` → 创建 `DragDropOperation`（Payload=SlotIndex）
  - `DragDropOperation.OnDrop` → 判断鼠标位置：
    - 在面板内 → `SwapItems(源Index, 目标Index)`
    - 在面板外 → `DropItem(源Index, 1)`

### 五十三、快捷栏扩展 10 格 + 快捷键映射
- **`QuickSlotComponent`**：`SlotCount 8→10`，新增 `SwapSlots`，`UseSlot` 使用后 `ClearSlot`
- **`WorldPlayerController`**：新增 `OnQuickSlotKeyPressed(SlotIndex)` → `UseSlot`
- 完成数据层准备，待搭建 `WBP_QuickSlotPanel`（T0 工作）

---

## 已完成工作（06/24 晚）

### 六十、物品分类系统（EItemCategory）
- **新增 `EItemCategory` 枚举**：Equipment / Consumable / Material / QuestItem
- **`ItemBase` 新增 `ItemCategory` 属性**：所有物品自带分类标记
- **容器准入规则**：快捷栏仅接受 `Consumable`，装备栏仅接受 `Equipment`，背包全品类
- **QuickSlotComponent**：`AssignSlot` / `SwapWithInventory` 增加分类检查
- **EquipmentComponent**：`EquipItem` 增加分类检查
- **`UConsumableItem` 构造器设默认 `Consumable`**：解决消耗品无法放入快捷栏的问题
- 涉及文件：`DataDefinitions.h`, `ItemBase.h/.cpp`, `EquipItem.h/.cpp`, `ConsumableItem.h/.cpp`, `QuickSlotComponent.h/.cpp`, `EquipmentComponent.h/.cpp`

### 六十一、物品数据表系统（4 表分离）
- **新建 `Data/ItemDataTable.h`**：4 个独立行结构体
  - `FEquipmentDataRow`：装备（EquipSlot / WeaponUsage / 五行加成等）
  - `FConsumableDataRow`：消耗品（EffectType / EffectValue）
  - `FMaterialDataRow`：材料（仅基础字段）
  - `FQuestItemDataRow`：任务物品（仅基础字段）
- **新建 `Data/ItemFactory.h/.cpp`**：`CreateEquipment` / `CreateConsumable` / `CreateMaterial` / `CreateQuestItem`
- **删除旧 `FItemDataRow` 和 `InitializeFromRow`**：结构体宏展开为手写字段
- **全字段中文 DisplayName**：ItemDataTable / ItemBase / EquipItem / ConsumableItem / EquipmentData
- **`EWeaponUsage` 新增 `None`**：防具饰品统一默认不设武器类型
- **`EEquipmentSlot` 新增 `Cloak`**：槽位 13 → 14
- 涉及文件：`ItemDataTable.h`, `ItemFactory.h/.cpp`, `ItemBase.h/.cpp`, `EquipItem.h/.cpp`, `ConsumableItem.h/.cpp`, `EquipmentData.h`

### 六十二、五维属性蓝图接口
- **`AttributeComponent` 新增 5 个 `BlueprintPure` 函数**：`GetJinLi` / `GetQiXue` / `GetNeiXi` / `GetShenFa` / `GetTiPo`
- 蓝图中可直接调用获取五维值，无需手动计算

### 六十三、装备栏刷新事件 + SwapWithInventory 返回 bool
- **`EquipmentComponent` 新增 `OnEquipmentChanged` 事件**：装备穿/卸时广播
- **`SwapWithInventory` 改为 `bool` 返回**：拖拽被拒绝（如武器→快捷栏）时蓝图可捕获失败
- **`InventoryComponent` 新增 `NotifyInventoryChanged()`**：蓝图中手动刷新背包显示
- 涉及文件：`EquipmentComponent.h/.cpp`, `QuickSlotComponent.h/.cpp`, `InventoryComponent.h/.cpp`

### 六十四、披风槽位
- **`EEquipmentSlot` 新增 `Cloak`**：槽位 13 → 14
- **`BaseCharacter` 新增 `CloakItem` Debug 属性**：与现有装备测试模式一致

## 已完成工作（06/25）

### 六十五、背包↔装备栏完整拖拽交互
- **`HandleSlotDrop` 统一路由函数**：Inventory/QuickSlot/EquipSlot 三容器拖拽逻辑集中到 C++，蓝图每个 Slot 的 OnDrop 只需一行调用
- **`UnequipToInventorySlot(EquipSlot, InvIdx)`**：装备拖回背包时支持三种行为：
  - 目标格空 → 直接放入
  - 目标格有同部位装备 → 互换
  - 目标格有不同类物品 → 拒绝（装备留在原位）
- **`CanEquipItemAtSlot` 戒指互通**：物品 `Slot=Ring` 可装备到 Ring1/Ring2 任一槽位
- **`EquipSlot→EquipSlot` 互换**：`MoveEquippedItem(From, To)` 直接在 TMap 中交换两槽位物品，不进背包
- **`EquipItem(Item, TargetSlotOverride)`**：新增目标槽参数，支持戒指互放
- **`EquipSlot→QuickSlot` 拒绝时刷新**：装备栏 UI 恢复 Icon
- 涉及文件：`EquipmentComponent.h/.cpp`, `DragDropHandler.h/.cpp`

### 六十六、HandleDragCancelled 统一取消拖拽逻辑
- **`HandleDragCancelled`**：检测鼠标下是否有任意 UMG，有则刷新来源容器后回弹，无则执行丢弃
- 三个 Slot 的 OnDragCancelled 蓝图统一为 1 个函数调用，无需维护 Panel 范围检测
- 涉及文件：`DragDropHandler.h/.cpp`

### 六十七、添加物品到背包统一接口
- **`ItemFactory::AddItemToInventory(RowID, Category, Count)`**：根据分类自动加载对应 DataTable → 创建物品 → 加入背包
- 蓝图中一行调用即可从数据表创建物品并放入背包
- 涉及文件：`ItemFactory.h/.cpp`

### 六十八、过量 Debug 日志清理
- 移除 `InventoryComponent`、`QuickSlotComponent`、`WorldPlayerController` 中信息性日志
- 保留错误/警告日志和装备拖拽 `[装备拖拽]` 调试日志

## 🎯 下一步待办

### P3 系统完善
- 地面物品拾取交互（点击/靠近触发 `OnPickup`）
- 右键菜单条件显示（使用/装备/拆分按钮可见性）
- 商店系统

### UMG 蓝图收尾
- `WBP_ItemContextMenu`：右键菜单条件显示
- `WBP_CharacterPanel`：角色面板属性刷新

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
- 背包拖拽丢弃（OnDragCancelled）调用的 `DropItem(SlotIndex)` 默认 Count=1，需手动改为 `DropItem(Slot, GetCountAt(Slot))` 才能丢弃全部

## 对话备注
- **Moliser3**：项目拥有者
- **小C**：AI游戏开发助手 （XiaoC_Role.md）

