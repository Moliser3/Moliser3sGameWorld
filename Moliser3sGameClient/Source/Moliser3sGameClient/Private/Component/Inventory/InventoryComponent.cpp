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

void UInventoryComponent::NotifyInventoryChanged()
{
	BroadcastChange();
}

bool UInventoryComponent::AddItem(UItemBase* Item, int32 Count)
{
	if (!Item || Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] AddItem 失败: 物品无效或数量<=0"));
		return false;
	}

	if (Item->MaxStackSize > 1)
	{
		int32 Remaining = Count;
		for (int32 i = 0; i < Items.Num() && Remaining > 0; i++)
		{
			if (Items[i] && Items[i]->ItemID == Item->ItemID && ItemCounts[i] < Item->MaxStackSize)
			{
				int32 Space = Item->MaxStackSize - ItemCounts[i];
				int32 Add = FMath::Min(Remaining, Space);
				ItemCounts[i] += Add;
				Remaining -= Add;
			}
		}
		if (Remaining <= 0)
		{
			BroadcastChange();
			return true;
		}

		while (Remaining > 0)
		{
			int32 EmptySlot = FindEmptySlot();
			if (EmptySlot == INDEX_NONE)
			{
				UE_LOG(LogTemp, Warning, TEXT("[背包] AddItem: %s 部分放入，背包已满"), *Item->ItemID.ToString());
				BroadcastChange();
				return true;
			}

			int32 Add = FMath::Min(Remaining, Item->MaxStackSize);
			if (EmptySlot == Items.Num())
			{
				Items.Add(Item);
				ItemCounts.Add(Add);
			}
			else
			{
				Items[EmptySlot] = Item;
				ItemCounts[EmptySlot] = Add;
			}
			Remaining -= Add;
		}

		BroadcastChange();
		return true;
	}

	int32 EmptySlot = FindEmptySlot();
	if (EmptySlot == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] AddItem 失败: %s 背包已满"), *Item->ItemID.ToString());
		return false;
	}

	if (EmptySlot == Items.Num())
	{
		Items.Add(Item);
		ItemCounts.Add(Count);
	}
	else
	{
		Items[EmptySlot] = Item;
		ItemCounts[EmptySlot] = Count;
	}

	BroadcastChange();
	return true;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Count)
{
	if (!Items.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] RemoveItem 失败: SlotIndex=%d 无效"), SlotIndex);
		return false;
	}
	if (!Items[SlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] RemoveItem 失败: Slot[%d] 为空"), SlotIndex);
		return false;
	}

	int32 RemoveCount = FMath::Min(Count, ItemCounts[SlotIndex]);
	ItemCounts[SlotIndex] -= RemoveCount;
	if (ItemCounts[SlotIndex] <= 0)
	{
		Items[SlotIndex] = nullptr;
		ItemCounts[SlotIndex] = 0;
	}

	BroadcastChange();
	return true;
}

UItemBase* UInventoryComponent::GetItemAt(int32 SlotIndex) const
{
	return Items.IsValidIndex(SlotIndex) ? Items[SlotIndex].Get() : nullptr;
}

int32 UInventoryComponent::GetCountAt(int32 SlotIndex) const
{
	return Items.IsValidIndex(SlotIndex) ? ItemCounts[SlotIndex] : 0;
}

void UInventoryComponent::SetItemAt(int32 SlotIndex, UItemBase* Item, int32 Count)
{
	if (SlotIndex >= MaxSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] SetItemAt 失败: SlotIndex=%d 超出 MaxSlots=%d"), SlotIndex, MaxSlots);
		return;
	}

	while (Items.Num() <= SlotIndex)
	{
		Items.Add(nullptr);
		ItemCounts.Add(0);
	}

	Items[SlotIndex] = Item;
	ItemCounts[SlotIndex] = Count;
	BroadcastChange();
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	int32 TotalCount = 0;
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i] && Items[i]->ItemID == ItemID)
			TotalCount += ItemCounts[i];
	}
	return TotalCount;
}

bool UInventoryComponent::IsFull() const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (!Items[i]) return false;
	}
	return Items.Num() >= MaxSlots;
}

void UInventoryComponent::DropItem(int32 SlotIndex, int32 Count)
{
	if (!Items.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] DropItem 失败: SlotIndex=%d 无效"), SlotIndex);
		return;
	}
	if (!Items[SlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] DropItem 失败: Slot[%d] 为空"), SlotIndex);
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	int32 DropCount = FMath::Min(Count, ItemCounts[SlotIndex]);

	FVector DropLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.0f;
	SpawnWorldItem(DropLocation, Items[SlotIndex].Get());

	ItemCounts[SlotIndex] -= DropCount;
	if (ItemCounts[SlotIndex] <= 0)
	{
		Items[SlotIndex] = nullptr;
		ItemCounts[SlotIndex] = 0;
	}
	BroadcastChange();
}

void UInventoryComponent::SwapItems(int32 IndexA, int32 IndexB)
{
	if (!Items.IsValidIndex(IndexA))
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] SwapItems 失败: IndexA=%d 无效"), IndexA);
		return;
	}
	if (IndexB >= MaxSlots || IndexA >= MaxSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] SwapItems 失败: 越界 IndexA=%d IndexB=%d"), IndexA, IndexB);
		return;
	}

	while (Items.Num() <= IndexB)
	{
		Items.Add(nullptr);
		ItemCounts.Add(0);
	}

	TObjectPtr<UItemBase> TempItem = Items[IndexA];
	int32 TempCount = ItemCounts[IndexA];
	Items[IndexA] = Items[IndexB];
	ItemCounts[IndexA] = ItemCounts[IndexB];
	Items[IndexB] = TempItem;
	ItemCounts[IndexB] = TempCount;

	BroadcastChange();
}

bool UInventoryComponent::TryStackOrSwap(int32 SourceSlot, int32 TargetSlot)
{
	if (!Items.IsValidIndex(SourceSlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] TryStackOrSwap 失败: Source=%d 无效"), SourceSlot);
		return false;
	}
	if (SourceSlot >= MaxSlots || TargetSlot >= MaxSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] TryStackOrSwap 失败: 越界 Source=%d Target=%d"), SourceSlot, TargetSlot);
		return false;
	}

	while (Items.Num() <= TargetSlot)
	{
		Items.Add(nullptr);
		ItemCounts.Add(0);
	}

	if (!Items[SourceSlot])
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] TryStackOrSwap: 源格为空"));
		return false;
	}

	UItemBase* SrcItem = Items[SourceSlot].Get();
	UItemBase* DstItem = Items[TargetSlot].Get();
	int32 SrcCount = ItemCounts[SourceSlot];
	int32 DstCount = ItemCounts[TargetSlot];

	if (DstItem && SrcItem->ItemID == DstItem->ItemID && DstCount < DstItem->MaxStackSize)
	{
		int32 Space = DstItem->MaxStackSize - DstCount;
		int32 MoveCount = FMath::Min(SrcCount, Space);

		ItemCounts[SourceSlot] -= MoveCount;
		ItemCounts[TargetSlot] += MoveCount;

		if (ItemCounts[SourceSlot] <= 0)
		{
			Items[SourceSlot] = nullptr;
			ItemCounts[SourceSlot] = 0;
		}

		BroadcastChange();
		return true;
	}

	SwapItems(SourceSlot, TargetSlot);
	return true;
}

void UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Items.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] UseItem 失败: SlotIndex=%d 无效"), SlotIndex);
		return;
	}
	if (!Items[SlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] UseItem 失败: Slot[%d] 为空"), SlotIndex);
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	Items[SlotIndex]->Use(Owner);

	ItemCounts[SlotIndex]--;
	if (ItemCounts[SlotIndex] <= 0)
	{
		Items[SlotIndex] = nullptr;
		ItemCounts[SlotIndex] = 0;
	}
	BroadcastChange();
}

bool UInventoryComponent::SplitItem(int32 SourceSlotIndex, int32 SplitCount)
{
	if (!Items.IsValidIndex(SourceSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] SplitItem 失败: SourceSlotIndex=%d 无效"), SourceSlotIndex);
		return false;
	}
	if (!Items[SourceSlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] SplitItem 失败: SourceSlot[%d] 为空"), SourceSlotIndex);
		return false;
	}
	if (SplitCount <= 0 || SplitCount >= ItemCounts[SourceSlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] SplitItem 失败: 拆分数量 %d 不合法 (当前=%d)"), SplitCount, ItemCounts[SourceSlotIndex]);
		return false;
	}

	UItemBase* SourceItem = Items[SourceSlotIndex].Get();

	int32 TargetSlot = FindEmptySlot();
	if (TargetSlot == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[背包] SplitItem 失败: 无可用空格"));
		return false;
	}

	if (TargetSlot == Items.Num())
	{
		Items.Add(SourceItem);
		ItemCounts.Add(0);
	}
	else
	{
		Items[TargetSlot] = SourceItem;
		ItemCounts[TargetSlot] = 0;
	}

	ItemCounts[SourceSlotIndex] -= SplitCount;
	ItemCounts[TargetSlot] += SplitCount;

	if (ItemCounts[SourceSlotIndex] <= 0)
	{
		Items[SourceSlotIndex] = nullptr;
		ItemCounts[SourceSlotIndex] = 0;
	}

	BroadcastChange();
	return true;
}

int32 UInventoryComponent::FindStackableSlot(FName ItemID, int32 MaxStack) const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i] && Items[i]->ItemID == ItemID && ItemCounts[i] < MaxStack)
			return i;
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
		return Items.Num();
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
