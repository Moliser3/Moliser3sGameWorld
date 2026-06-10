// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 配置敌人角色的移动参数
	// bUseControllerDesiredRotation = true 让 Controller 控制旋转
	// 敌人由 AAIController 驱动，朝向由 AI 决策决定
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		MoveComp->MaxWalkSpeed = GetRunSpeed();
		MoveComp->bUseControllerDesiredRotation = true;
	}
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
}