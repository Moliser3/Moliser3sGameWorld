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
