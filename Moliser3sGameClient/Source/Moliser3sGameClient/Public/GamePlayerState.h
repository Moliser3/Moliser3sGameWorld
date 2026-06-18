#pragma once

#include "CoreMinimal.h"
#include "GamePlayerState.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Default          UMETA(DisplayName = "默认"),
	BattlePerception UMETA(DisplayName = "战斗感知")
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	Idle    UMETA(DisplayName = "闲置"),
	Walking UMETA(DisplayName = "行走"),
	Running UMETA(DisplayName = "奔跑"),
	Skill   UMETA(DisplayName = "释放技能")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, ECombatState, NewCombat);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionStateChanged, EActionState, NewAction);
