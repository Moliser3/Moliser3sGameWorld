#pragma once

#include "CoreMinimal.h"
#include "UI/ItemDragDropOperation.h"
#include "Data/EquipmentData.h"
#include "DragDropHandler.generated.h"

class APlayerCharacter;
class UItemDragDropOperation;

UCLASS()
class MOLISER3SGAMECLIENT_API UDragDropHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "拖拽", meta = (WorldContext = "WorldContextObject"))
	static bool HandleSlotDrop(
		const UObject* WorldContextObject,
		UItemDragDropOperation* DragOp,
		ESlotContainerType TargetContainer,
		int32 TargetSlotIndex,
		EEquipmentSlot TargetEquipSlot
	);

	UFUNCTION(BlueprintCallable, Category = "拖拽", meta = (WorldContext = "WorldContextObject"))
	static void HandleDragCancelled(
		const UObject* WorldContextObject,
		UItemDragDropOperation* DragOp
	);
};
