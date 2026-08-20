#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentData.h"
#include "EquipmentComponent.generated.h"

class UEquipItem;
class UAttributeComponent;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentComponent();

	UFUNCTION(BlueprintCallable, Category = "装备")
	bool EquipItem(UEquipItem* Item, EEquipmentSlot TargetSlotOverride);

	UFUNCTION(BlueprintCallable, Category = "装备")
	bool UnequipItem(EEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "装备")
	bool UnequipToInventory(EEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "装备")
	bool UnequipToInventorySlot(EEquipmentSlot EquipSlot, int32 InventorySlotIndex);

	UFUNCTION(BlueprintCallable, Category = "装备")
	void DropEquippedItem(EEquipmentSlot Slot);

	UFUNCTION(BlueprintPure, Category = "装备")
	static bool CanEquipItemAtSlot(UEquipItem* Item, EEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintPure, Category = "装备")
	UEquipItem* GetEquippedItem(EEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "装备")
	TArray<UEquipItem*> GetAllEquippedItems() const;

	UFUNCTION(BlueprintPure, Category = "装备")
	bool IsSlotLocked(EEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "装备")
	TArray<EEquipmentSlot> GetEmptySlots() const;

	UFUNCTION(BlueprintPure, Category = "装备")
	TArray<EEquipmentSlot> GetOccupiedSlots() const;

	void MoveEquippedItem(EEquipmentSlot From, EEquipmentSlot To);

	UPROPERTY(BlueprintAssignable, Category = "装备")
	FOnEquipmentChanged OnEquipmentChanged;

	EEquipmentSlot FindEquippedSlot(UEquipItem* Item) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "装备")
	TMap<EEquipmentSlot, TObjectPtr<UEquipItem>> EquippedItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "装备")
	TSet<EEquipmentSlot> LockedSlots;

	void ApplyItemBonuses(UEquipItem* Item);
	void RemoveItemBonuses(UEquipItem* Item);
	UAttributeComponent* GetAttributeComp() const;
};
