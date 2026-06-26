#include "Component/Equipment/EquipmentComponent.h"
#include "Component/Inventory/InventoryComponent.h"
#include "Data/EquipItem.h"
#include "Component/Attribute/AttributeComponent.h"
#include "WorldActors/WorldItemActor.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UEquipmentComponent::EquipItem(UEquipItem* Item, EEquipmentSlot TargetSlotOverride)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem 失败: 物品为空"));
		return false;
	}

	if (Item->ItemCategory != EItemCategory::Equipment)
	{
		UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem 拒绝: 分类不是 Equipment"));
		return false;
	}

	EEquipmentSlot TargetSlot = TargetSlotOverride;

	// 戒指物品（Slot=Ring）可以装备到 Ring1 或 Ring2
	if (Item->Slot == EEquipmentSlot::Ring)
	{
		if (TargetSlot != EEquipmentSlot::Ring1 && TargetSlot != EEquipmentSlot::Ring2)
		{
			UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem 拒绝: 戒指只能装备到戒指槽"));
			return false;
		}
	}
	else if (Item->Slot == EEquipmentSlot::Ring1 || Item->Slot == EEquipmentSlot::Ring2)
	{
		// 兼容旧数据：Ring1/Ring2 也可互放
		if (TargetSlot != EEquipmentSlot::Ring1 && TargetSlot != EEquipmentSlot::Ring2)
			return false;
	}
	else if (Item->Slot != TargetSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem 拒绝: 物品槽位 %d 与目标 %d 不匹配"), (int32)Item->Slot, (int32)TargetSlot);
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem 开始: Item=%s 源Slot=%d 目标Slot=%d"),
		*Item->ItemID.ToString(), (int32)Item->Slot, (int32)TargetSlot);

	if (LockedSlots.Contains(TargetSlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem 拒绝: 槽位被锁定 Slot=%d"), (int32)TargetSlot);
		return false;
	}

	if (Item->bIsOffHand())
	{
		if (TObjectPtr<UEquipItem>* MainWeapon = EquippedItems.Find(EEquipmentSlot::MainHand))
		{
			if (*MainWeapon && (*MainWeapon)->bIsTwoHanded())
			{
				UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem 拒绝: 主手为双手武器，不能装备副手"));
				return false;
			}
		}
	}

	if (EquippedItems.Contains(TargetSlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem: 槽位已有装备，卸下旧装备并放回背包"));
		UnequipToInventory(TargetSlot);
	}

	if (Item->bIsTwoHanded())
	{
		if (EquippedItems.Contains(EEquipmentSlot::OffHand))
			UnequipToInventory(EEquipmentSlot::OffHand);
		LockedSlots.Add(EEquipmentSlot::OffHand);
	}

	EquippedItems.Add(TargetSlot, Item);
	ApplyItemBonuses(Item);

	OnEquipmentChanged.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] EquipItem 成功: %s 装备到 Slot=%d"), *Item->ItemID.ToString(), (int32)TargetSlot);
	return true;
}

bool UEquipmentComponent::UnequipItem(EEquipmentSlot Slot)
{
	TObjectPtr<UEquipItem>* ItemPtr = EquippedItems.Find(Slot);
	if (!ItemPtr || !*ItemPtr) return false;

	UEquipItem* Item = *ItemPtr;

	if (Item->bIsTwoHanded())
		LockedSlots.Remove(EEquipmentSlot::OffHand);

	RemoveItemBonuses(Item);
	EquippedItems.Remove(Slot);
	OnEquipmentChanged.Broadcast();
	return true;
}

bool UEquipmentComponent::UnequipToInventory(EEquipmentSlot Slot)
{
	TObjectPtr<UEquipItem>* ItemPtr = EquippedItems.Find(Slot);
	if (!ItemPtr || !*ItemPtr) return false;

	UEquipItem* Item = *ItemPtr;

	AActor* Owner = GetOwner();
	if (!Owner) return false;

	UInventoryComponent* Inv = Owner->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return false;

	if (Item->bIsTwoHanded())
		LockedSlots.Remove(EEquipmentSlot::OffHand);

	RemoveItemBonuses(Item);
	EquippedItems.Remove(Slot);

	Inv->AddItem(Item);

	OnEquipmentChanged.Broadcast();
	return true;
}

bool UEquipmentComponent::UnequipToInventorySlot(EEquipmentSlot EquipSlot, int32 InventorySlotIndex)
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	UInventoryComponent* Inv = Owner->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return false;

	TObjectPtr<UEquipItem>* EquipPtr = EquippedItems.Find(EquipSlot);
	if (!EquipPtr || !*EquipPtr) return false;

	UEquipItem* EquipItem = EquipPtr->Get();
	UItemBase* InvItem = Inv->GetItemAt(InventorySlotIndex);
	UEquipItem* InvEquipItem = Cast<UEquipItem>(InvItem);

	// 背包格有同部位装备 → 交换
	if (InvEquipItem && CanEquipItemAtSlot(InvEquipItem, EquipSlot))
	{
		RemoveItemBonuses(EquipItem);
		RemoveItemBonuses(InvEquipItem);

		EquippedItems.Remove(EquipSlot);
		Inv->SetItemAt(InventorySlotIndex, EquipItem);

		EquippedItems.Add(EquipSlot, InvEquipItem);
		ApplyItemBonuses(InvEquipItem);

		OnEquipmentChanged.Broadcast();
		return true;
	}

	// 背包格空 → 直接放入
	if (!InvItem)
	{
		RemoveItemBonuses(EquipItem);
		EquippedItems.Remove(EquipSlot);
		Inv->SetItemAt(InventorySlotIndex, EquipItem);
		OnEquipmentChanged.Broadcast();
		return true;
	}

	// 背包格有非同类物品 → 拒绝
	return false;
}

void UEquipmentComponent::DropEquippedItem(EEquipmentSlot Slot)
{
	TObjectPtr<UEquipItem>* ItemPtr = EquippedItems.Find(Slot);
	if (!ItemPtr || !*ItemPtr) return;

	UEquipItem* Item = *ItemPtr;

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
		WorldItem->InitializeFromItem(Item);

	if (Item->bIsTwoHanded())
		LockedSlots.Remove(EEquipmentSlot::OffHand);

	RemoveItemBonuses(Item);
	EquippedItems.Remove(Slot);
	OnEquipmentChanged.Broadcast();
}

bool UEquipmentComponent::CanEquipItemAtSlot(UEquipItem* Item, EEquipmentSlot TargetSlot)
{
	if (!Item) return false;
	if (Item->ItemCategory != EItemCategory::Equipment) return false;

	// 戒指（Ring/Ring1/Ring2）可装备到 Ring1 或 Ring2
	if (Item->Slot == EEquipmentSlot::Ring || Item->Slot == EEquipmentSlot::Ring1 || Item->Slot == EEquipmentSlot::Ring2)
		return TargetSlot == EEquipmentSlot::Ring1 || TargetSlot == EEquipmentSlot::Ring2;

	return Item->Slot == TargetSlot;
}

UEquipItem* UEquipmentComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	const TObjectPtr<UEquipItem>* ItemPtr = EquippedItems.Find(Slot);
	return ItemPtr ? ItemPtr->Get() : nullptr;
}

EEquipmentSlot UEquipmentComponent::FindEquippedSlot(UEquipItem* Item) const
{
	for (const auto& Pair : EquippedItems)
	{
		if (Pair.Value == Item)
			return Pair.Key;
	}
	return EEquipmentSlot::MainHand;
}

void UEquipmentComponent::MoveEquippedItem(EEquipmentSlot From, EEquipmentSlot To)
{
	if (From == To) return;

	TObjectPtr<UEquipItem>* SrcPtr = EquippedItems.Find(From);
	if (!SrcPtr || !*SrcPtr) return;

	UEquipItem* SrcItem = SrcPtr->Get();
	UEquipItem* DstItem = nullptr;
	if (TObjectPtr<UEquipItem>* DstPtr = EquippedItems.Find(To))
		DstItem = DstPtr->Get();

	RemoveItemBonuses(SrcItem);
	if (DstItem)
		RemoveItemBonuses(DstItem);

	EquippedItems.Remove(From);
	EquippedItems.Remove(To);

	EquippedItems.Add(To, SrcItem);
	if (DstItem)
		EquippedItems.Add(From, DstItem);

	if (TObjectPtr<UEquipItem>* NewA = EquippedItems.Find(From)) { if (*NewA) ApplyItemBonuses(NewA->Get()); }
	if (TObjectPtr<UEquipItem>* NewB = EquippedItems.Find(To))   { if (*NewB) ApplyItemBonuses(NewB->Get()); }

	OnEquipmentChanged.Broadcast();
}

TArray<UEquipItem*> UEquipmentComponent::GetAllEquippedItems() const
{
	TArray<UEquipItem*> Result;
	for (const auto& Pair : EquippedItems)
	{
		if (Pair.Value)
			Result.Add(Pair.Value.Get());
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
		EEquipmentSlot::Pants,     EEquipmentSlot::Boots,     EEquipmentSlot::Cloak,
		EEquipmentSlot::Amulet,    EEquipmentSlot::Ring1,     EEquipmentSlot::Ring2,
		EEquipmentSlot::MainHand,  EEquipmentSlot::OffHand
	};

	TArray<EEquipmentSlot> Empty;
	for (EEquipmentSlot Slot : AllSlots)
	{
		if (!EquippedItems.Contains(Slot) && !LockedSlots.Contains(Slot))
			Empty.Add(Slot);
	}
	return Empty;
}

TArray<EEquipmentSlot> UEquipmentComponent::GetOccupiedSlots() const
{
	TArray<EEquipmentSlot> Occupied;
	for (const auto& Pair : EquippedItems)
		Occupied.Add(Pair.Key);
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
