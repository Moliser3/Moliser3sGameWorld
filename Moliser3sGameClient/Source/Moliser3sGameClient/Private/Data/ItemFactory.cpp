#include "Data/ItemFactory.h"
#include "Data/ItemDataTable.h"
#include "Data/ItemBase.h"
#include "Data/EquipItem.h"
#include "Data/ConsumableItem.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

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
