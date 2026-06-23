#include "Component/Inventory/InventoryComponent.h"
#include "Data/ItemBase.h"
#include "WorldActors/WorldItemActor.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BroadcastChange()
{
	if (!bBatchMode)
		OnInventoryChanged.Broadcast();
}

void UInventoryComponent::BeginBatch()
{
	bBatchMode = true;
}

void UInventoryComponent::EndBatch()
{
	bBatchMode = false;
	OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::AddItem(UItemBase* Item)
{
	if (!Item) return false;

	if (Item->MaxStackSize > 1)
	{
		int32 StackSlot = FindStackableSlot(Item->ItemID, Item->MaxStackSize);
		if (StackSlot != INDEX_NONE)
		{
			BroadcastChange();
			return true;
		}
	}

	int32 EmptySlot = FindEmptySlot();
	if (EmptySlot == INDEX_NONE) return false;

	if (EmptySlot == Items.Num())
		Items.Add(Item);
	else
		Items[EmptySlot] = Item;

	BroadcastChange();
	return true;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Count)
{
	if (!Items.IsValidIndex(SlotIndex) || !Items[SlotIndex]) return false;

	Items.RemoveAt(SlotIndex);
	Items.Add(nullptr);

	BroadcastChange();
	return true;
}

UItemBase* UInventoryComponent::GetItemAt(int32 SlotIndex) const
{
	return Items.IsValidIndex(SlotIndex) ? Items[SlotIndex].Get() : nullptr;
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	int32 Count = 0;
	for (const auto& Item : Items)
	{
		if (Item && Item->ItemID == ItemID)
			Count++;
	}
	return Count;
}

bool UInventoryComponent::IsFull() const
{
	return FindEmptySlot() == INDEX_NONE;
}

void UInventoryComponent::DropItem(int32 SlotIndex, int32 Count)
{
	if (!Items.IsValidIndex(SlotIndex) || !Items[SlotIndex]) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector DropLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.0f;

	SpawnWorldItem(DropLocation, Items[SlotIndex].Get());
	RemoveItem(SlotIndex, Count);
}

void UInventoryComponent::SwapItems(int32 IndexA, int32 IndexB)
{
	UE_LOG(LogTemp, Warning, TEXT("[Swap] 进入 A=%d B=%d Items.Num=%d MaxSlots=%d"),
		IndexA, IndexB, Items.Num(), MaxSlots);

	if (!Items.IsValidIndex(IndexA))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Swap] IndexA=%d 无效! Items.Num=%d"), IndexA, Items.Num());
		return;
	}
	if (IndexB >= MaxSlots || IndexA >= MaxSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Swap] 越界! IndexA=%d IndexB=%d MaxSlots=%d"), IndexA, IndexB, MaxSlots);
		return;
	}

	while (Items.Num() <= IndexB)
		Items.Add(nullptr);

	UE_LOG(LogTemp, Warning, TEXT("[Swap] 执行交换: Items[%d]=%s <-> Items[%d]=%s"),
		IndexA, Items[IndexA] ? *Items[IndexA]->ItemID.ToString() : TEXT("null"),
		IndexB, Items[IndexB] ? *Items[IndexB]->ItemID.ToString() : TEXT("null"));

	TObjectPtr<UItemBase> Temp = Items[IndexA];
	Items[IndexA] = Items[IndexB];
	Items[IndexB] = Temp;

	BroadcastChange();
	UE_LOG(LogTemp, Warning, TEXT("[Swap] 完成"));
}

void UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Items.IsValidIndex(SlotIndex) || !Items[SlotIndex]) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	Items[SlotIndex]->Use(Owner);

	BroadcastChange();
}

int32 UInventoryComponent::FindStackableSlot(FName ItemID, int32 MaxStack) const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i] && Items[i]->ItemID == ItemID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::FindEmptySlot() const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (!Items[i])
			return i;
	}
	if (Items.Num() < MaxSlots)
	{
		return Items.Num();
	}
	return INDEX_NONE;
}

TArray<UItemBase*> UInventoryComponent::GetAllItems() const
{
	TArray<UItemBase*> Result;
	for (const auto& Item : Items)
	{
		if (Item)
			Result.Add(Item.Get());
	}
	return Result;
}

AWorldItemActor* UInventoryComponent::SpawnWorldItem(const FVector& Location, UItemBase* Item)
{
	if (!Item) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters Params;
	Params.Owner = nullptr;

	AWorldItemActor* WorldItem = World->SpawnActor<AWorldItemActor>(
		Location, FRotator::ZeroRotator, Params
	);

	if (WorldItem)
	{
		WorldItem->InitializeFromItem(Item);
	}

	return WorldItem;
}
