// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGameMode.h"
#include "PlayerCharacter.h"
#include "WorldPlayerController.h"

AWorldGameMode::AWorldGameMode()
{
	// 设置默认 Pawn 为玩家角色
	DefaultPawnClass = APlayerCharacter::StaticClass();

	// 设置默认 PlayerController
	PlayerControllerClass = AWorldPlayerController::StaticClass();
}