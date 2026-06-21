#include "Component/Equipment/EquipmentComponent.h"
#include "Data/EquipItem.h"
#include "Component/Attribute/AttributeComponent.h"
#include "GameFramework/Actor.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UEquipmentComponent::EquipItem(UEquipItem* Item)
{
	if (!Item)
	{
		return false;
	}

	EEquipmentSlot TargetSlot = Item->Slot;

	// 检查槽位是否被锁定
	if (LockedSlots.Contains(TargetSlot))
	{
		return false;
	}

	// 如果是副手物品，检查主手是否装备了双手武器
	if (Item->bIsOffHand())
	{
		if (TObjectPtr<UEquipItem>* MainWeapon = EquippedItems.Find(EEquipmentSlot::MainHand))
		{
			if (*MainWeapon && (*MainWeapon)->bIsTwoHanded())
			{
				return false;
			}
		}
	}

	// 如果该槽位已装备，先卸下
	if (EquippedItems.Contains(TargetSlot))
	{
		UnequipItem(TargetSlot);
	}

	// 如果是双手武器，锁定副手槽
	if (Item->bIsTwoHanded())
	{
		// 副手有物品则先卸下
		if (EquippedItems.Contains(EEquipmentSlot::OffHand))
		{
			UnequipItem(EEquipmentSlot::OffHand);
		}
		LockedSlots.Add(EEquipmentSlot::OffHand);
	}

	// 装备物品
	EquippedItems.Add(TargetSlot, Item);
	ApplyItemBonuses(Item);

	return true;
}

bool UEquipmentComponent::UnequipItem(EEquipmentSlot Slot)
{
	TObjectPtr<UEquipItem>* ItemPtr = EquippedItems.Find(Slot);
	if (!ItemPtr || !*ItemPtr)
	{
		return false;
	}

	UEquipItem* Item = *ItemPtr;

	// 如果是双手武器，解锁副手槽
	if (Item->bIsTwoHanded())
	{
		LockedSlots.Remove(EEquipmentSlot::OffHand);
	}

	// 移除加成
	RemoveItemBonuses(Item);

	// 清空槽位
	EquippedItems.Remove(Slot);

	return true;
}

UEquipItem* UEquipmentComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	const TObjectPtr<UEquipItem>* ItemPtr = EquippedItems.Find(Slot);
	return ItemPtr ? ItemPtr->Get() : nullptr;
}

TArray<UEquipItem*> UEquipmentComponent::GetAllEquippedItems() const
{
	TArray<UEquipItem*> Result;
	for (const auto& Pair : EquippedItems)
	{
		if (Pair.Value)
		{
			Result.Add(Pair.Value.Get());
		}
	}
	return Result;
}

bool UEquipmentComponent::IsSlotLocked(EEquipmentSlot Slot) const
{
	return LockedSlots.Contains(Slot);
}

TArray<EEquipmentSlot> UEquipmentComponent::GetEmptySlots() const
{
	TArray<EEquipmentSlot> AllSlots = {
		EEquipmentSlot::Helmet,    EEquipmentSlot::Shoulders, EEquipmentSlot::Chest,
		EEquipmentSlot::Bracers,   EEquipmentSlot::Gloves,    EEquipmentSlot::Belt,
		EEquipmentSlot::Pants,     EEquipmentSlot::Boots,     EEquipmentSlot::Amulet,
		EEquipmentSlot::Ring1,     EEquipmentSlot::Ring2,     EEquipmentSlot::MainHand,
		EEquipmentSlot::OffHand
	};

	TArray<EEquipmentSlot> Empty;
	for (EEquipmentSlot Slot : AllSlots)
	{
		if (!EquippedItems.Contains(Slot) && !LockedSlots.Contains(Slot))
		{
			Empty.Add(Slot);
		}
	}
	return Empty;
}

TArray<EEquipmentSlot> UEquipmentComponent::GetOccupiedSlots() const
{
	TArray<EEquipmentSlot> Occupied;
	for (const auto& Pair : EquippedItems)
	{
		Occupied.Add(Pair.Key);
	}
	return Occupied;
}

void UEquipmentComponent::GetTotalWuXingBonuses(int32& OutJin, int32& OutMu, int32& OutShui, int32& OutHuo, int32& OutTu) const
{
	OutJin = OutMu = OutShui = OutHuo = OutTu = 0;
	for (const auto& Pair : EquippedItems)
	{
		if (Pair.Value)
		{
			OutJin  += Pair.Value->JinBonus;
			OutMu   += Pair.Value->MuBonus;
			OutShui += Pair.Value->ShuiBonus;
			OutHuo  += Pair.Value->HuoBonus;
			OutTu   += Pair.Value->TuBonus;
		}
	}
}

void UEquipmentComponent::ApplyItemBonuses(UEquipItem* Item)
{
	if (!Item) return;

	UAttributeComponent* AttrComp = GetAttributeComp();
	if (!AttrComp) return;

	// 直接修改 CharacterData 的五行值（加法）
	FCharacterCoreData Data = AttrComp->GetCharacterData();
	Data.AddEquipmentBonus(Item->JinBonus, Item->MuBonus, Item->ShuiBonus, Item->HuoBonus, Item->TuBonus);
	AttrComp->SetCharacterData(Data);
}

void UEquipmentComponent::RemoveItemBonuses(UEquipItem* Item)
{
	if (!Item) return;

	UAttributeComponent* AttrComp = GetAttributeComp();
	if (!AttrComp) return;

	FCharacterCoreData Data = AttrComp->GetCharacterData();
	Data.RemoveEquipmentBonus(Item->JinBonus, Item->MuBonus, Item->ShuiBonus, Item->HuoBonus, Item->TuBonus);
	AttrComp->SetCharacterData(Data);
}

UAttributeComponent* UEquipmentComponent::GetAttributeComp() const
{
	AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UAttributeComponent>() : nullptr;
}
