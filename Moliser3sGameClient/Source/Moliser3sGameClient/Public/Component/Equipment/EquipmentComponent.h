#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentData.h"
#include "EquipmentComponent.generated.h"

class UEquipItem;
class UAttributeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentComponent();

	UFUNCTION(BlueprintCallable, Category = "装备")
	bool EquipItem(UEquipItem* Item);

	UFUNCTION(BlueprintCallable, Category = "装备")
	bool UnequipItem(EEquipmentSlot Slot);

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

	UFUNCTION(BlueprintPure, Category = "装备")
	void GetTotalWuXingBonuses(int32& OutJin, int32& OutMu, int32& OutShui, int32& OutHuo, int32& OutTu) const;

	UPROPERTY(BlueprintAssignable, Category = "装备")
	FOnEquipmentChanged OnEquipmentChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "装备")
	TMap<EEquipmentSlot, TObjectPtr<UEquipItem>> EquippedItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "装备")
	TSet<EEquipmentSlot> LockedSlots;

	void ApplyItemBonuses(UEquipItem* Item);
	void RemoveItemBonuses(UEquipItem* Item);
	UAttributeComponent* GetAttributeComp() const;
};
