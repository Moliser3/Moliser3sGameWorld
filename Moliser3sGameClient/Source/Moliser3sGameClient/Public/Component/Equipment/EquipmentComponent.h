#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentData.h"
#include "EquipmentComponent.generated.h"

class UEquipItem;
class UAttributeComponent;

/**
 * 装备管理组件
 * 管理14个槽位的装备/卸下，处理双手武器槽位锁定，应用五行加成
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentComponent();

	/** 装备物品到对应槽位 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItem(UEquipItem* Item);

	/** 从指定槽位卸下物品 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UnequipItem(EEquipmentSlot Slot);

	/** 获取指定槽位的装备 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	UEquipItem* GetEquippedItem(EEquipmentSlot Slot) const;

	/** 获取所有已装备物品 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<UEquipItem*> GetAllEquippedItems() const;

	/** 槽位是否被锁定（双手武器锁定副手） */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsSlotLocked(EEquipmentSlot Slot) const;

	/** 获取所有空槽位 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<EEquipmentSlot> GetEmptySlots() const;

	/** 获取所有已占用槽位 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<EEquipmentSlot> GetOccupiedSlots() const;

	/** 计算所有装备的五行加成总和 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	void GetTotalWuXingBonuses(int32& OutJin, int32& OutMu, int32& OutShui, int32& OutHuo, int32& OutTu) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TMap<EEquipmentSlot, TObjectPtr<UEquipItem>> EquippedItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TSet<EEquipmentSlot> LockedSlots;

	/** 向 AttributeComponent 应用物品五行加成 */
	void ApplyItemBonuses(UEquipItem* Item);

	/** 从 AttributeComponent 移除物品五行加成 */
	void RemoveItemBonuses(UEquipItem* Item);

	/** 获取所属角色的 AttributeComponent */
	UAttributeComponent* GetAttributeComp() const;
};
