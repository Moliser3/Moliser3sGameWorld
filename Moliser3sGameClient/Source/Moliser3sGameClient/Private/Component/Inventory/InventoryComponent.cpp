#include "Component/Inventory/InventoryComponent.h"
#include "Data/ItemBase.h"
#include "WorldActors/WorldItemActor.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItem(UItemBase* Item)
{
	if (!Item) return false;

	// 尝试堆叠
	if (Item->MaxStackSize > 1)
	{
		int32 StackSlot = FindStackableSlot(Item->ItemID, Item->MaxStackSize);
		if (StackSlot != INDEX_NONE)
		{
			OnInventoryChanged.Broadcast();
			return true;
		}
	}

	// 找空位
	int32 EmptySlot = FindEmptySlot();
	if (EmptySlot == INDEX_NONE) return false;

	if (EmptySlot == Items.Num())
		Items.Add(Item);
	else
		Items[EmptySlot] = Item;

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Count)
{
	if (!Items.IsValidIndex(SlotIndex) || !Items[SlotIndex]) return false;

	Items.RemoveAt(SlotIndex);
	Items.Add(nullptr);

	OnInventoryChanged.Broadcast();
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

void UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Items.IsValidIndex(SlotIndex) || !Items[SlotIndex]) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	Items[SlotIndex]->Use(Owner);

	OnInventoryChanged.Broadcast();
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
