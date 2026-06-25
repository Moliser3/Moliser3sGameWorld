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
	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] AssignSlot: Slot[%d] <== %s x%d"), Index,
		Item ? *Item->ItemID.ToString() : TEXT("null"), Count);
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
	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] ClearSlot: Slot[%d] %s 已清除"), Index,
		Slots[Index] ? *Slots[Index]->ItemID.ToString() : TEXT("空"));
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

	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] UseSlot: Slot[%d] 使用 %s (剩余 %d)"), Index,
		*Slots[Index]->ItemID.ToString(), ItemCounts[Index]);

	Slots[Index]->Use(Owner);

	ItemCounts[Index]--;
	if (ItemCounts[Index] <= 0)
	{
		Slots[Index] = nullptr;
		ItemCounts[Index] = 0;
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] UseSlot: Slot[%d] 物品消耗完毕，已清空"), Index);
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

	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapSlots: Slot[%d]=%s(x%d) <-> Slot[%d]=%s(x%d)"),
		IndexA, Slots[IndexA] ? *Slots[IndexA]->ItemID.ToString() : TEXT("空"), ItemCounts[IndexA],
		IndexB, Slots[IndexB] ? *Slots[IndexB]->ItemID.ToString() : TEXT("空"), ItemCounts[IndexB]);

	TObjectPtr<UItemBase> TempItem = Slots[IndexA];
	int32 TempCount = ItemCounts[IndexA];
	Slots[IndexA] = Slots[IndexB];
	ItemCounts[IndexA] = ItemCounts[IndexB];
	Slots[IndexB] = TempItem;
	ItemCounts[IndexB] = TempCount;

	OnQuickSlotChanged.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapSlots 完成"));
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

	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] DropSlotItem: Slot[%d] %s x%d 丢弃到场景"), Index,
		*Slots[Index]->ItemID.ToString(), ItemCounts[Index]);

	FActorSpawnParameters Params;
	Params.Owner = nullptr;

	AWorldItemActor* WorldItem = World->SpawnActor<AWorldItemActor>(
		DropLocation, FRotator::ZeroRotator, Params
	);
	if (WorldItem)
	{
		WorldItem->InitializeFromItem(Slots[Index].Get());
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] DropSlotItem: WorldItemActor 已生成"));
	}

	Slots[Index] = nullptr;
	ItemCounts[Index] = 0;
	OnQuickSlotChanged.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] DropSlotItem: Slot[%d] 已清空"), Index);
}

bool UQuickSlotComponent::SwapWithInventory(int32 InventorySlotIndex, int32 QuickSlotIndex, bool bFromInventory)
{
	AActor* Owner = GetOwner();
	if (!Owner) { return false; }

	UInventoryComponent* Inv = Owner->FindComponentByClass<UInventoryComponent>();
	if (!Inv)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory ❌ 失败: 未找到 InventoryComponent"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] === SwapWithInventory 开始: %s → 背包[%d] <-> 快捷栏[%d] ==="),
		bFromInventory ? TEXT("背包") : TEXT("快捷栏"), InventorySlotIndex, QuickSlotIndex);

	UItemBase* ItemA = Inv->GetItemAt(InventorySlotIndex);
	UItemBase* ItemB = GetSlotItem(QuickSlotIndex);
	int32 CountA = Inv->GetCountAt(InventorySlotIndex);
	int32 CountB = GetCountAt(QuickSlotIndex);

	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory: 背包[%d]=%s(%sx%d) 快捷栏[%d]=%s(%sx%d)"),
		InventorySlotIndex,
		ItemA ? TEXT("有") : TEXT("空"), ItemA ? *ItemA->ItemID.ToString() : TEXT(""), CountA,
		QuickSlotIndex,
		ItemB ? TEXT("有") : TEXT("空"), ItemB ? *ItemB->ItemID.ToString() : TEXT(""), CountB);

	// 检查放入快捷栏的物品是否为消耗品
	if (ItemA && ItemA->ItemCategory != EItemCategory::Consumable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory 失败: %s 不是消耗品"), *ItemA->ItemID.ToString());
		return false;
	}

	Inv->BeginBatch();

	// ── 同类物品叠加 ──
	if (ItemA && ItemB && ItemA->ItemID == ItemB->ItemID)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory: 同类物品 %s, 尝试叠加..."), *ItemA->ItemID.ToString());

		if (bFromInventory)
		{
			if (CountB < ItemB->MaxStackSize)
			{
				int32 Space = ItemB->MaxStackSize - CountB;
				int32 MoveCount = FMath::Min(CountA, Space);
				Inv->RemoveItem(InventorySlotIndex, MoveCount);
				ItemCounts[QuickSlotIndex] += MoveCount;
				UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory ✅: %s 从背包叠加 %d 个到快捷栏 (背包剩余=%d 快捷栏=%d)"),
					*ItemB->ItemID.ToString(), MoveCount, Inv->GetCountAt(InventorySlotIndex), ItemCounts[QuickSlotIndex]);
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
				UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory ✅: %s 从快捷栏叠加 %d 个到背包 (快捷栏=%d 背包=%d)"),
					*ItemA->ItemID.ToString(), MoveCount, ItemCounts[QuickSlotIndex], CountA + MoveCount);
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
				UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory ✅: %s 从快捷栏叠加 %d 个到背包 (快捷栏=%d 背包=%d)"),
					*ItemA->ItemID.ToString(), MoveCount, ItemCounts[QuickSlotIndex], CountA + MoveCount);
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
				UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory ✅: %s 从背包叠加 %d 个到快捷栏 (背包剩余=%d 快捷栏=%d)"),
					*ItemB->ItemID.ToString(), MoveCount, Inv->GetCountAt(InventorySlotIndex), ItemCounts[QuickSlotIndex]);
				OnQuickSlotChanged.Broadcast();
				Inv->EndBatch();
				return true;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory: 两格均已满，退化为交换"),
			CountA, ItemA->MaxStackSize, CountB, ItemB->MaxStackSize);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SwapWithInventory: 不同物品或单边为空，执行交换"));
	}

	if (ItemA) Inv->RemoveItem(InventorySlotIndex, CountA);
	if (ItemB) ClearSlot(QuickSlotIndex);
	if (ItemA) AssignSlot(QuickSlotIndex, ItemA, CountA);
	if (ItemB) Inv->SetItemAt(InventorySlotIndex, ItemB, CountB);

	Inv->EndBatch();
	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] === SwapWithInventory 完成 ==="));
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
	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] === SplitItem 开始: SourceSlot=%d SplitCount=%d ==="), SourceSlotIndex, SplitCount);
	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem: Slots.Num=%d SlotCount=%d"), Slots.Num(), SlotCount);

	if (!Slots.IsValidIndex(SourceSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem ❌ 失败: SourceSlotIndex=%d 无效"), SourceSlotIndex);
		return false;
	}
	if (!Slots[SourceSlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem ❌ 失败: SourceSlot[%d] 为空"), SourceSlotIndex);
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem: 源物品=%s Count=%d MaxStack=%d"),
		*Slots[SourceSlotIndex]->ItemID.ToString(), ItemCounts[SourceSlotIndex], Slots[SourceSlotIndex]->MaxStackSize);

	if (SplitCount <= 0 || SplitCount >= ItemCounts[SourceSlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem ❌ 失败: 拆分数量 %d 不合法 (当前=%d)"), SplitCount, ItemCounts[SourceSlotIndex]);
		return false;
	}

	UItemBase* SourceItem = Slots[SourceSlotIndex].Get();

	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem: 查找空格..."));
	int32 TargetSlot = INDEX_NONE;
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (!Slots[i])
		{
			TargetSlot = i;
			UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem: 找到空格 Slot[%d]"), TargetSlot);
			break;
		}
	}
	if (TargetSlot == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem ❌ 失败: 无可用空格"));
		return false;
	}
	Slots[TargetSlot] = SourceItem;
	ItemCounts[TargetSlot] = 0;

	ItemCounts[SourceSlotIndex] -= SplitCount;
	ItemCounts[TargetSlot] += SplitCount;

	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem ✅: %s 从 Slot[%d] 拆分 %d 个到 Slot[%d] (源剩余=%d 目标=%d)"),
		*SourceItem->ItemID.ToString(), SourceSlotIndex, SplitCount, TargetSlot,
		ItemCounts[SourceSlotIndex], ItemCounts[TargetSlot]);

	if (ItemCounts[SourceSlotIndex] <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[快捷栏] SplitItem: 源格已空，清空 Slot[%d]"), SourceSlotIndex);
		Slots[SourceSlotIndex] = nullptr;
		ItemCounts[SourceSlotIndex] = 0;
	}

	OnQuickSlotChanged.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("[快捷栏] === SplitItem 完成 ==="));
	return true;
}
