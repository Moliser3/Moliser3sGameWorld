#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

class UItemBase;

UENUM(BlueprintType)
enum class ESlotContainerType : uint8
{
	Inventory UMETA(DisplayName = "背包"),
	QuickSlot UMETA(DisplayName = "快捷栏")
};

UCLASS(BlueprintType)
class MOLISER3SGAMECLIENT_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "拖拽")
	ESlotContainerType SourceContainer;

	UPROPERTY(BlueprintReadWrite, Category = "拖拽")
	int32 SourceSlotIndex = -1;

	UPROPERTY(BlueprintReadWrite, Category = "拖拽")
	TObjectPtr<UItemBase> DraggedItem;
};
