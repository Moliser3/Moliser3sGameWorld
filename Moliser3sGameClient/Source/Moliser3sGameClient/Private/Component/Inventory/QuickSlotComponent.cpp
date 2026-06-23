#include "Component/Inventory/QuickSlotComponent.h"
#include "Data/ItemBase.h"

UQuickSlotComponent::UQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Slots.SetNum(SlotCount);
}

bool UQuickSlotComponent::AssignSlot(int32 Index, UItemBase* Item)
{
	if (!Slots.IsValidIndex(Index)) return false;
	Slots[Index] = Item;
	OnQuickSlotChanged.Broadcast();
	return true;
}

void UQuickSlotComponent::ClearSlot(int32 Index)
{
	if (!Slots.IsValidIndex(Index)) return;
	Slots[Index] = nullptr;
	OnQuickSlotChanged.Broadcast();
}

void UQuickSlotComponent::UseSlot(int32 Index)
{
	if (!Slots.IsValidIndex(Index) || !Slots[Index]) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	Slots[Index]->Use(Owner);

	Slots[Index] = nullptr;
	OnQuickSlotChanged.Broadcast();
}

void UQuickSlotComponent::SwapSlots(int32 IndexA, int32 IndexB)
{
	if (!Slots.IsValidIndex(IndexA) || !Slots.IsValidIndex(IndexB)) return;

	TObjectPtr<UItemBase> Temp = Slots[IndexA];
	Slots[IndexA] = Slots[IndexB];
	Slots[IndexB] = Temp;

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

UItemBase* UQuickSlotComponent::GetSlotItem(int32 Index) const
{
	return Slots.IsValidIndex(Index) ? Slots[Index].Get() : nullptr;
}
