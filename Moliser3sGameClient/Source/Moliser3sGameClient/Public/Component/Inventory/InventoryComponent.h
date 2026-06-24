#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItemBase;
class AWorldItemActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "背包", meta = (ClampMin = "1"))
	int32 MaxSlots = 30;

	UPROPERTY(BlueprintAssignable, Category = "背包")
	FOnInventoryChanged OnInventoryChanged;

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool AddItem(UItemBase* Item, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool RemoveItem(int32 SlotIndex, int32 Count = 1);

	UFUNCTION(BlueprintPure, Category = "背包")
	UItemBase* GetItemAt(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetCountAt(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "背包")
	bool IsFull() const;

	UFUNCTION(BlueprintPure, Category = "背包")
	int32 GetSlotCount() const { return Items.Num(); }

	UFUNCTION(BlueprintPure, Category = "背包")
	TArray<UItemBase*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category = "背包")
	void DropItem(int32 SlotIndex, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "背包")
	void UseItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "背包")
	void SetItemAt(int32 SlotIndex, UItemBase* Item, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "背包")
	void SwapItems(int32 IndexA, int32 IndexB);

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool TryStackOrSwap(int32 SourceSlot, int32 TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "背包")
	bool SplitItem(int32 SourceSlotIndex, int32 SplitCount);

	UFUNCTION(BlueprintCallable, Category = "背包")
	void BeginBatch();

	UFUNCTION(BlueprintCallable, Category = "背包")
	void EndBatch();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "背包")
	TArray<TObjectPtr<UItemBase>> Items;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "背包")
	TArray<int32> ItemCounts;

	bool bBatchMode = false;

	void BroadcastChange();
	int32 FindStackableSlot(FName ItemID, int32 MaxStack) const;
	int32 FindEmptySlot() const;
	AWorldItemActor* SpawnWorldItem(const FVector& Location, UItemBase* Item);
};
