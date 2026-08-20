#pragma once

#include "CoreMinimal.h"
#include "DataDefinitions.generated.h"

/** 物品分类 */
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Equipment  UMETA(DisplayName = "装备"),
	Consumable UMETA(DisplayName = "消耗品"),
	Material   UMETA(DisplayName = "材料"),
	QuestItem  UMETA(DisplayName = "任务物品")
};
