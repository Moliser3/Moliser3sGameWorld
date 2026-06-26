#include "Component/Inventory/QuickSlotComponent.h"
#include "Component/Inventory/InventoryComponent.h"
#include "Data/ItemBase.h"
#include "WorldActors/WorldItemActor.h"
#include "Engine/World.h"

UQuickSlotComponent::UQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Slots.SetNum(SlotCount);
	ItemCounts.Init(0, SlotCount);
}

bool UQuickSlotComponent::AssignSlot(int32 Index, UItemBase* Item, int32 Count)
{
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] AssignSlot 失败: Index=%d 无效"), Index);
		return false;
	}
	if (Item && Item->ItemCategory != EItemCategory::Consumable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] AssignSlot 失败: %s 不是消耗品"), *Item->ItemID.ToString());
		return false;
	}
	Slots[Index] = Item;
	ItemCounts[Index] = Count;
	OnQuickSlotChanged.Broadcast();
	return true;
}

void UQuickSlotComponent::ClearSlot(int32 Index)
{
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] ClearSlot 失败: Index=%d 无效"), Index);
		return;
	}
	Slots[Index] = nullptr;
	ItemCounts[Index] = 0;
	OnQuickSlotChanged.Broadcast();
}

void UQuickSlotComponent::UseSlot(int32 Index)
{
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] UseSlot 失败: Index=%d 无效"), Index);
		return;
	}
	if (!Slots[Index])
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] UseSlot 失败: Slot[%d] 为空"), Index);
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	Slots[Index]->Use(Owner);

	ItemCounts[Index]--;
	if (ItemCounts[Index] <= 0)
	{
		Slots[Index] = nullptr;
		ItemCounts[Index] = 0;
	}

	OnQuickSlotChanged.Broadcast();
}

void UQuickSlotComponent::SwapSlots(int32 IndexA, int32 IndexB)
{
	if (!Slots.IsValidIndex(IndexA) || !Slots.IsValidIndex(IndexB))
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapSlots 失败: IndexA=%d IndexB=%d 无效"), IndexA, IndexB);
		return;
	}

	TObjectPtr<UItemBase> TempItem = Slots[IndexA];
	int32 TempCount = ItemCounts[IndexA];
	Slots[IndexA] = Slots[IndexB];
	ItemCounts[IndexA] = ItemCounts[IndexB];
	Slots[IndexB] = TempItem;
	ItemCounts[IndexB] = TempCount;

	OnQuickSlotChanged.Broadcast();
}

bool UQuickSlotComponent::TryStackOrSwap(int32 SourceSlot, int32 TargetSlot)
{
	if (!Slots.IsValidIndex(SourceSlot) || !Slots.IsValidIndex(TargetSlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] TryStackOrSwap 失败: 索引无效 Source=%d Target=%d"), SourceSlot, TargetSlot);
		return false;
	}
	if (!Slots[SourceSlot])
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] TryStackOrSwap: 源格为空"));
		return false;
	}

	UItemBase* SrcItem = Slots[SourceSlot].Get();
	UItemBase* DstItem = Slots[TargetSlot].Get();
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
			Slots[SourceSlot] = nullptr;
			ItemCounts[SourceSlot] = 0;
		}

		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] TryStackOrSwap: %s 从 Slot[%d] 叠加 %d 个到 Slot[%d] (目标=%d 源剩余=%d)"),
			*SrcItem->ItemID.ToString(), SourceSlot, MoveCount, TargetSlot, ItemCounts[TargetSlot], ItemCounts[SourceSlot]);
		OnQuickSlotChanged.Broadcast();
		return true;
	}

	SwapSlots(SourceSlot, TargetSlot);
	return true;
}

void UQuickSlotComponent::NotifyQuickSlotChanged()
{
	OnQuickSlotChanged.Broadcast();
}

TArray<UItemBase*> UQuickSlotComponent::GetAllSlots() const
{
	TArray<UItemBase*> Result;
	for (const auto& Slot : Slots)
	{
		if (Slot)
			Result.Add(Slot.Get());
	}
	return Result;
}

void UQuickSlotComponent::DropSlotItem(int32 Index)
{
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] DropSlotItem 失败: Index=%d 无效"), Index);
		return;
	}
	if (!Slots[Index])
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] DropSlotItem 失败: Slot[%d] 为空"), Index);
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	UWorld* World = Owner->GetWorld();
	if (!World) return;

	FVector DropLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.0f;

	FActorSpawnParameters Params;
	Params.Owner = nullptr;

	AWorldItemActor* WorldItem = World->SpawnActor<AWorldItemActor>(
		DropLocation, FRotator::ZeroRotator, Params
	);
	if (WorldItem)
		WorldItem->InitializeFromItem(Slots[Index].Get());

	Slots[Index] = nullptr;
	ItemCounts[Index] = 0;
	OnQuickSlotChanged.Broadcast();
}

bool UQuickSlotComponent::SwapWithInventory(int32 InventorySlotIndex, int32 QuickSlotIndex, bool bFromInventory)
{
	AActor* Owner = GetOwner();
	if (!Owner) { return false; }

	UInventoryComponent* Inv = Owner->FindComponentByClass<UInventoryComponent>();
	if (!Inv)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory 失败: 未找到 InventoryComponent"));
		return false;
	}

	UItemBase* ItemA = Inv->GetItemAt(InventorySlotIndex);
	UItemBase* ItemB = GetSlotItem(QuickSlotIndex);
	int32 CountA = Inv->GetCountAt(InventorySlotIndex);
	int32 CountB = GetCountAt(QuickSlotIndex);

	if (ItemA && ItemA->ItemCategory != EItemCategory::Consumable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory 拒绝: %s 不是消耗品"), *ItemA->ItemID.ToString());
		return false;
	}

	Inv->BeginBatch();

	if (ItemA && ItemB && ItemA->ItemID == ItemB->ItemID)
	{
		if (bFromInventory)
		{
			if (CountB < ItemB->MaxStackSize)
			{
				int32 Space = ItemB->MaxStackSize - CountB;
				int32 MoveCount = FMath::Min(CountA, Space);
				Inv->RemoveItem(InventorySlotIndex, MoveCount);
				ItemCounts[QuickSlotIndex] += MoveCount;
				OnQuickSlotChanged.Broadcast();
				Inv->EndBatch();
				return true;
			}
			if (CountA < ItemA->MaxStackSize)
			{
				int32 Space = ItemA->MaxStackSize - CountA;
				int32 MoveCount = FMath::Min(CountB, Space);
				ItemCounts[QuickSlotIndex] -= MoveCount;
				Inv->SetItemAt(InventorySlotIndex, ItemA, CountA + MoveCount);
				if (ItemCounts[QuickSlotIndex] <= 0) { Slots[QuickSlotIndex] = nullptr; ItemCounts[QuickSlotIndex] = 0; }
				OnQuickSlotChanged.Broadcast();
				Inv->EndBatch();
				return true;
			}
		}
		else
		{
			if (CountA < ItemA->MaxStackSize)
			{
				int32 Space = ItemA->MaxStackSize - CountA;
				int32 MoveCount = FMath::Min(CountB, Space);
				ItemCounts[QuickSlotIndex] -= MoveCount;
				Inv->SetItemAt(InventorySlotIndex, ItemA, CountA + MoveCount);
				if (ItemCounts[QuickSlotIndex] <= 0) { Slots[QuickSlotIndex] = nullptr; ItemCounts[QuickSlotIndex] = 0; }
				OnQuickSlotChanged.Broadcast();
				Inv->EndBatch();
				return true;
			}
			if (CountB < ItemB->MaxStackSize)
			{
				int32 Space = ItemB->MaxStackSize - CountB;
				int32 MoveCount = FMath::Min(CountA, Space);
				Inv->RemoveItem(InventorySlotIndex, MoveCount);
				ItemCounts[QuickSlotIndex] += MoveCount;
				OnQuickSlotChanged.Broadcast();
				Inv->EndBatch();
				return true;
			}
		}
	}

	if (ItemA) Inv->RemoveItem(InventorySlotIndex, CountA);
	if (ItemB) ClearSlot(QuickSlotIndex);
	if (ItemA) AssignSlot(QuickSlotIndex, ItemA, CountA);
	if (ItemB) Inv->SetItemAt(InventorySlotIndex, ItemB, CountB);

	Inv->EndBatch();
	return true;
}

UItemBase* UQuickSlotComponent::GetSlotItem(int32 Index) const
{
	return Slots.IsValidIndex(Index) ? Slots[Index].Get() : nullptr;
}

int32 UQuickSlotComponent::GetCountAt(int32 Index) const
{
	return Slots.IsValidIndex(Index) ? ItemCounts[Index] : 0;
}

bool UQuickSlotComponent::SplitItem(int32 SourceSlotIndex, int32 SplitCount)
{
	if (!Slots.IsValidIndex(SourceSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem 失败: SourceSlotIndex=%d 无效"), SourceSlotIndex);
		return false;
	}
	if (!Slots[SourceSlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem 失败: SourceSlot[%d] 为空"), SourceSlotIndex);
		return false;
	}
	if (SplitCount <= 0 || SplitCount >= ItemCounts[SourceSlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem 失败: 拆分数量 %d 不合法 (当前=%d)"), SplitCount, ItemCounts[SourceSlotIndex]);
		return false;
	}

	UItemBase* SourceItem = Slots[SourceSlotIndex].Get();

	int32 TargetSlot = INDEX_NONE;
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (!Slots[i])
		{
			TargetSlot = i;
			break;
		}
	}
	if (TargetSlot == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem 失败: 无可用空格"));
		return false;
	}
	Slots[TargetSlot] = SourceItem;
	ItemCounts[TargetSlot] = 0;

	ItemCounts[SourceSlotIndex] -= SplitCount;
	ItemCounts[TargetSlot] += SplitCount;

	if (ItemCounts[SourceSlotIndex] <= 0)
	{
		Slots[SourceSlotIndex] = nullptr;
		ItemCounts[SourceSlotIndex] = 0;
	}

	OnQuickSlotChanged.Broadcast();
	return true;
}
