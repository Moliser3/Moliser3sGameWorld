#pragma once

#include "CoreMinimal.h"
#include "DataDefinitions.generated.h"

/** 五行枚举 */
UENUM(BlueprintType)
enum class EWuXing : uint8
{
	Jin   UMETA(DisplayName = "金"),
	Mu    UMETA(DisplayName = "木"),
	Shui  UMETA(DisplayName = "水"),
	Huo   UMETA(DisplayName = "火"),
	Tu    UMETA(DisplayName = "土")
};

/** 物品分类 */
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Equipment  UMETA(DisplayName = "装备"),
	Consumable UMETA(DisplayName = "消耗品"),
	Material   UMETA(DisplayName = "材料"),
	QuestItem  UMETA(DisplayName = "任务物品")
};
