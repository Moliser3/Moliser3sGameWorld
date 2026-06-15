// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建朝向控制组件（先于移动参数配置）
	FacingComponent = CreateDefaultSubobject<UFacingComponent>(TEXT("FacingComponent"));

	// 创建技能系统组件
	SkillSystemComponent = CreateDefaultSubobject<USkillSystemComponent>(TEXT("SkillSystemComponent"));

	// 创建相机组件（不挂在任何组件下，由 CameraControllerComponent 控制位置和旋转）
	// 相机完全独立于角色坐标系，不会继承任何轴向旋转
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

	// 配置玩家角色的移动参数
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		MoveComp->bUseControllerDesiredRotation = false;

		// 初始速度为奔跑速度
		MoveComp->MaxWalkSpeed = GetRunSpeed();

		// 跳跃参数
		MoveComp->JumpZVelocity = 420.0f;
		MoveComp->AirControl = 0.3f;
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 监听朝向模式改变事件，自动切换速度
	if (FacingComponent)
	{
		FacingComponent->OnFacingModeChanged.AddDynamic(this, &APlayerCharacter::UpdateMovementSpeed);
	}

}

void APlayerCharacter::SetAimingFullSpeed(bool bFullSpeed)
{
	bAimingFullSpeed = bFullSpeed;
}

void APlayerCharacter::UpdateMovementSpeed(EFacingMode NewMode)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// 速度由 Controller 根据上下文控制，朝向模式切换时默认重置为行走速度
		MoveComp->MaxWalkSpeed = GetWalkSpeed();
	}
	if (NewMode != EFacingMode::Aiming)
	{
		bAimingFullSpeed = false;
	}
}
