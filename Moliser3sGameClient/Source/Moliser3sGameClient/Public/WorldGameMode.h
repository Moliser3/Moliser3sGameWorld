// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WorldGameMode.generated.h"

/**
 * 世界游戏模式
 * 设置默认 Pawn 为 APlayerCharacter
 */
UCLASS()
class MOLISER3SGAMECLIENT_API AWorldGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AWorldGameMode();
};
