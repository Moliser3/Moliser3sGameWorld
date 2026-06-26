#include "UI/DragDropHandler.h"
#include "UI/ItemDragDropOperation.h"
#include "Data/ItemBase.h"
#include "Data/EquipItem.h"
#include "Component/Inventory/InventoryComponent.h"
#include "Component/Inventory/QuickSlotComponent.h"
#include "Component/Equipment/EquipmentComponent.h"
#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SViewport.h"
#include "Engine/GameViewportClient.h"

bool UDragDropHandler::HandleSlotDrop(
	const UObject* WorldContextObject,
	UItemDragDropOperation* DragOp,
	ESlotContainerType TargetContainer,
	int32 TargetSlotIndex,
	EEquipmentSlot TargetEquipSlot
)
{
	if (!DragOp) return false;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return false;

	APlayerCharacter* Player = Cast<APlayerCharacter>(World->GetFirstPlayerController()->GetPawn());
	if (!Player) return false;

	UInventoryComponent* Inv = Player->GetInventory();
	UQuickSlotComponent* QS = Player->GetQuickSlot();
	UEquipmentComponent* Equip = Player->GetEquipmentComponent();
	if (!Inv || !QS || !Equip) return false;

	ESlotContainerType Source = DragOp->SourceContainer;
	int32 SourceIdx = DragOp->SourceSlotIndex;
	UItemBase* DraggedItem = DragOp->DraggedItem;
	UEquipItem* EquipItemCast = Cast<UEquipItem>(DraggedItem);

	// ── 来源: 背包 ──
	if (Source == ESlotContainerType::Inventory)
	{
		switch (TargetContainer)
		{
		case ESlotContainerType::Inventory:
			if (SourceIdx >= 0 && TargetSlotIndex >= 0)
				Inv->TryStackOrSwap(SourceIdx, TargetSlotIndex);
			return true;

		case ESlotContainerType::QuickSlot:
			if (SourceIdx >= 0 && TargetSlotIndex >= 0)
			{
				if (!QS->SwapWithInventory(SourceIdx, TargetSlotIndex, true))
					Inv->NotifyInventoryChanged();
			}
			return true;

		case ESlotContainerType::EquipSlot:
		{
			UEquipItem* EquipCandidate = EquipItemCast;
			if (!EquipCandidate && SourceIdx >= 0)
				EquipCandidate = Cast<UEquipItem>(Inv->GetItemAt(SourceIdx));

			UE_LOG(LogTemp, Warning, TEXT("[装备拖拽] 来源=背包 Slot[%d] 目标=%s EquipCandidate=%s CanEquip=%s"),
				SourceIdx, *StaticEnum<EEquipmentSlot>()->GetValueAsString(TargetEquipSlot),
				EquipCandidate ? *EquipCandidate->ItemID.ToString() : TEXT("null"),
				(EquipCandidate && Equip->CanEquipItemAtSlot(EquipCandidate, TargetEquipSlot)) ? TEXT("是") : TEXT("否"));

			if (EquipCandidate && Equip->CanEquipItemAtSlot(EquipCandidate, TargetEquipSlot))
			{
				Inv->RemoveItem(SourceIdx);
				Equip->EquipItem(EquipCandidate, TargetEquipSlot);
			}
			Inv->NotifyInventoryChanged();
			return true;
		}
		}
		return true;
	}

	// ── 来源: 快捷栏 ──
	if (Source == ESlotContainerType::QuickSlot)
	{
		switch (TargetContainer)
		{
		case ESlotContainerType::Inventory:
			if (SourceIdx >= 0 && TargetSlotIndex >= 0)
			{
				if (!QS->SwapWithInventory(TargetSlotIndex, SourceIdx, false))
					QS->NotifyQuickSlotChanged();
			}
			return true;

		case ESlotContainerType::QuickSlot:
			if (SourceIdx >= 0 && TargetSlotIndex >= 0)
				QS->TryStackOrSwap(SourceIdx, TargetSlotIndex);
			return true;

		case ESlotContainerType::EquipSlot:
			QS->NotifyQuickSlotChanged();
			return true;
		}
		return true;
	}

	// ── 来源: 装备槽 ──
	if (Source == ESlotContainerType::EquipSlot)
	{
		switch (TargetContainer)
		{
		case ESlotContainerType::Inventory:
			if (EquipItemCast && TargetSlotIndex >= 0)
			{
				EEquipmentSlot ActualSlot = Equip->FindEquippedSlot(EquipItemCast);
				Equip->UnequipToInventorySlot(ActualSlot, TargetSlotIndex);
			}
			Equip->OnEquipmentChanged.Broadcast();
			Inv->NotifyInventoryChanged();
			return true;

		case ESlotContainerType::QuickSlot:
			Equip->OnEquipmentChanged.Broadcast();
			return true;

		case ESlotContainerType::EquipSlot:
		{
			UEquipItem* SrcItem = Cast<UEquipItem>(DragOp->DraggedItem);
			if (SrcItem && Equip->CanEquipItemAtSlot(SrcItem, TargetEquipSlot))
			{
				EEquipmentSlot SrcSlot = Equip->FindEquippedSlot(SrcItem);
				if (SrcSlot != TargetEquipSlot)
					Equip->MoveEquippedItem(SrcSlot, TargetEquipSlot);
			}
			Equip->OnEquipmentChanged.Broadcast();
			return true;
		}
		}
		Equip->OnEquipmentChanged.Broadcast();
		return true;
	}

	return true;
}

void UDragDropHandler::HandleDragCancelled(const UObject* WorldContextObject, UItemDragDropOperation* DragOp)
{
	if (!DragOp) return;

	// 鼠标在任意 UMG 上 → 不放回也不丢弃（回弹原格），刷新来源容器恢复 Icon
	FWidgetPath WidgetPath = FSlateApplication::Get().LocateWindowUnderMouse(
		FSlateApplication::Get().GetCursorPos(),
		FSlateApplication::Get().GetInteractiveTopLevelWindows()
	);
	for (int32 i = 0; i < WidgetPath.Widgets.Num(); i++)
	{
		if (WidgetPath.Widgets[i].Widget->GetType() == FName("SObjectWidget"))
		{
			UWorld* W = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
			if (APlayerCharacter* P = W ? Cast<APlayerCharacter>(W->GetFirstPlayerController()->GetPawn()) : nullptr)
			{
				switch (DragOp->SourceContainer)
				{
				case ESlotContainerType::Inventory:
					if (P->GetInventory()) P->GetInventory()->NotifyInventoryChanged();
					break;
				case ESlotContainerType::QuickSlot:
					if (P->GetQuickSlot()) P->GetQuickSlot()->NotifyQuickSlotChanged();
					break;
				case ESlotContainerType::EquipSlot:
					if (P->GetEquipmentComponent()) P->GetEquipmentComponent()->OnEquipmentChanged.Broadcast();
					break;
				}
			}
			return;
		}
	}

	// 鼠标在游戏场景上 → 执行丢弃
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(World->GetFirstPlayerController()->GetPawn());
	if (!Player) return;

	switch (DragOp->SourceContainer)
	{
	case ESlotContainerType::Inventory:
		if (Player->GetInventory() && DragOp->SourceSlotIndex >= 0)
			Player->GetInventory()->DropItem(DragOp->SourceSlotIndex, Player->GetInventory()->GetCountAt(DragOp->SourceSlotIndex));
		break;

	case ESlotContainerType::QuickSlot:
		if (Player->GetQuickSlot())
			Player->GetQuickSlot()->DropSlotItem(DragOp->SourceSlotIndex);
		break;

	case ESlotContainerType::EquipSlot:
	{
		UEquipItem* EquipItem = Cast<UEquipItem>(DragOp->DraggedItem);
		if (EquipItem && Player->GetEquipmentComponent())
		{
			EEquipmentSlot ActualSlot = Player->GetEquipmentComponent()->FindEquippedSlot(EquipItem);
			Player->GetEquipmentComponent()->DropEquippedItem(ActualSlot);
		}
		break;
	}
	}
}
