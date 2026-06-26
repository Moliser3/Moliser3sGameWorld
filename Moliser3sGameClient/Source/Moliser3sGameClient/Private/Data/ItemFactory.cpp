#include "Data/ItemFactory.h"
#include "Data/ItemDataTable.h"
#include "Data/ItemBase.h"
#include "Data/EquipItem.h"
#include "Data/ConsumableItem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacter.h"
#include "Component/Inventory/InventoryComponent.h"

static UItemBase* CreateItemInternal(const UObject* WorldContextObject, UItemBase* Item, FName RowID,
	FText DisplayName, FText Description, TSoftObjectPtr<UTexture2D> Icon,
	TSoftObjectPtr<UStaticMesh> WorldMesh, int32 MaxStackSize)
{
	if (!Item) return nullptr;

	Item->ItemID = RowID;
	Item->DisplayName = DisplayName;
	Item->Description = Description;
	Item->Icon = Icon;
	Item->WorldMesh = WorldMesh;
	Item->MaxStackSize = MaxStackSize;

	UE_LOG(LogTemp, Warning, TEXT("[物品工厂] ✅ 创建物品: %s (%s)"), *DisplayName.ToString(), *RowID.ToString());
	return Item;
}

UEquipItem* UItemFactory::CreateEquipment(const UObject* WorldContextObject, UDataTable* DataTable, FName RowID)
{
	if (!DataTable) { UE_LOG(LogTemp, Warning, TEXT("[物品工厂] 失败: DataTable 为空")); return nullptr; }

	FEquipmentDataRow* Row = DataTable->FindRow<FEquipmentDataRow>(RowID, TEXT("ItemFactory"));
	if (!Row) { UE_LOG(LogTemp, Warning, TEXT("[物品工厂] 失败: 未找到装备行 RowID=%s"), *RowID.ToString()); return nullptr; }

	UEquipItem* Item = NewObject<UEquipItem>(WorldContextObject ? WorldContextObject->GetWorld() : nullptr);
	CreateItemInternal(WorldContextObject, Item, RowID, Row->DisplayName, Row->Description,
		Row->Icon, Row->WorldMesh, Row->MaxStackSize);
	Item->ItemCategory = EItemCategory::Equipment;
	Item->Slot = Row->EquipSlot;
	Item->WeaponUsage = Row->WeaponUsage;
	Item->LevelRequirement = Row->LevelRequirement;
	Item->JinBonus = Row->JinBonus;
	Item->MuBonus = Row->MuBonus;
	Item->ShuiBonus = Row->ShuiBonus;
	Item->HuoBonus = Row->HuoBonus;
	Item->TuBonus = Row->TuBonus;
	return Item;
}

UConsumableItem* UItemFactory::CreateConsumable(const UObject* WorldContextObject, UDataTable* DataTable, FName RowID)
{
	if (!DataTable) { UE_LOG(LogTemp, Warning, TEXT("[物品工厂] 失败: DataTable 为空")); return nullptr; }

	FConsumableDataRow* Row = DataTable->FindRow<FConsumableDataRow>(RowID, TEXT("ItemFactory"));
	if (!Row) { UE_LOG(LogTemp, Warning, TEXT("[物品工厂] 失败: 未找到消耗品行 RowID=%s"), *RowID.ToString()); return nullptr; }

	UConsumableItem* Item = NewObject<UConsumableItem>(WorldContextObject ? WorldContextObject->GetWorld() : nullptr);
	CreateItemInternal(WorldContextObject, Item, RowID, Row->DisplayName, Row->Description,
		Row->Icon, Row->WorldMesh, Row->MaxStackSize);
	Item->ItemCategory = EItemCategory::Consumable;
	Item->EffectType = Row->EffectType;
	Item->EffectValue = Row->EffectValue;
	return Item;
}

UItemBase* UItemFactory::CreateMaterial(const UObject* WorldContextObject, UDataTable* DataTable, FName RowID)
{
	if (!DataTable) { UE_LOG(LogTemp, Warning, TEXT("[物品工厂] 失败: DataTable 为空")); return nullptr; }

	FMaterialDataRow* Row = DataTable->FindRow<FMaterialDataRow>(RowID, TEXT("ItemFactory"));
	if (!Row) { UE_LOG(LogTemp, Warning, TEXT("[物品工厂] 失败: 未找到材料行 RowID=%s"), *RowID.ToString()); return nullptr; }

	UItemBase* Item = NewObject<UItemBase>(WorldContextObject ? WorldContextObject->GetWorld() : nullptr);
	CreateItemInternal(WorldContextObject, Item, RowID, Row->DisplayName, Row->Description,
		Row->Icon, Row->WorldMesh, Row->MaxStackSize);
	Item->ItemCategory = EItemCategory::Material;
	return Item;
}

UItemBase* UItemFactory::CreateQuestItem(const UObject* WorldContextObject, UDataTable* DataTable, FName RowID)
{
	if (!DataTable) { UE_LOG(LogTemp, Warning, TEXT("[物品工厂] 失败: DataTable 为空")); return nullptr; }

	FQuestItemDataRow* Row = DataTable->FindRow<FQuestItemDataRow>(RowID, TEXT("ItemFactory"));
	if (!Row) { UE_LOG(LogTemp, Warning, TEXT("[物品工厂] 失败: 未找到任务物品行 RowID=%s"), *RowID.ToString()); return nullptr; }

	UItemBase* Item = NewObject<UItemBase>(WorldContextObject ? WorldContextObject->GetWorld() : nullptr);
	CreateItemInternal(WorldContextObject, Item, RowID, Row->DisplayName, Row->Description,
		Row->Icon, Row->WorldMesh, Row->MaxStackSize);
	Item->ItemCategory = EItemCategory::QuestItem;
	return Item;
}

static UDataTable* GetDataTableForCategory(EItemCategory Category)
{
	FString Path;
	switch (Category)
	{
	case EItemCategory::Equipment:  Path = TEXT("/Game/CoreSystem/Items/DT_Equipment.DT_Equipment"); break;
	case EItemCategory::Consumable: Path = TEXT("/Game/CoreSystem/Items/DT_Consumable.DT_Consumable"); break;
	case EItemCategory::Material:   Path = TEXT("/Game/CoreSystem/Items/DT_Material.DT_Material"); break;
	case EItemCategory::QuestItem:  Path = TEXT("/Game/CoreSystem/Items/DT_QuestItem.DT_QuestItem"); break;
	default: return nullptr;
	}
	return Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path));
}

bool UItemFactory::AddItemToInventory(const UObject* WorldContextObject, FName RowID, EItemCategory Category, int32 Count)
{
	UDataTable* DT = GetDataTableForCategory(Category);
	if (!DT)
	{
		UE_LOG(LogTemp, Warning, TEXT("[物品工厂] AddItemToInventory 失败: 无法加载分类 %d 的数据表"), (int32)Category);
		return false;
	}

	UItemBase* Item = nullptr;
	switch (Category)
	{
	case EItemCategory::Equipment:  Item = CreateEquipment(WorldContextObject, DT, RowID); break;
	case EItemCategory::Consumable: Item = CreateConsumable(WorldContextObject, DT, RowID); break;
	case EItemCategory::Material:   Item = CreateMaterial(WorldContextObject, DT, RowID); break;
	case EItemCategory::QuestItem:  Item = CreateQuestItem(WorldContextObject, DT, RowID); break;
	}

	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("[物品工厂] AddItemToInventory 失败: RowID=%s 未找到或创建失败"), *RowID.ToString());
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return false;

	APlayerCharacter* Player = Cast<APlayerCharacter>(World->GetFirstPlayerController()->GetPawn());
	if (!Player || !Player->GetInventory())
	{
		UE_LOG(LogTemp, Warning, TEXT("[物品工厂] AddItemToInventory 失败: 无法获取玩家背包"));
		return false;
	}

	if (!Player->GetInventory()->AddItem(Item, Count))
	{
		UE_LOG(LogTemp, Warning, TEXT("[物品工厂] AddItemToInventory 失败: 背包已满,无法添加 %s x%d"), *RowID.ToString(), Count);
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("[物品工厂] AddItemToInventory ✅: %s x%d 已添加到背包"), *RowID.ToString(), Count);
	return true;
}
