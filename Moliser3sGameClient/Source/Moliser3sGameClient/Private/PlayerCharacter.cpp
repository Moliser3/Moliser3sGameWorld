// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"
#include "Skill/MeleeSlashSkill.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建朝向控制组件（先于移动参数配置）
	FacingComponent = CreateDefaultSubobject<UFacingComponent>(TEXT("FacingComponent"));

	// 创建技能系统组件
	SkillSystemComponent = CreateDefaultSubobject<USkillSystemComponent>(TEXT("SkillSystemComponent"));

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

	// 注册默认技能：近战斩击
	if (SkillSystemComponent)
	{
		UMeleeSlashSkill* MeleeSlash = NewObject<UMeleeSlashSkill>(this);
		if (MeleeSlash)
		{
			MeleeSlash->Radius = 100.0f;
			MeleeSlash->HalfAngleDeg = 22.5f;
			MeleeSlash->BaseDamage = 5.0f;
			MeleeSlash->MaxZDiff = 150.0f;
			SkillSystemComponent->AddSkill(MeleeSlash);
		}
	}
}

void APlayerCharacter::UpdateMovementSpeed(EFacingMode NewMode)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (NewMode == EFacingMode::Aiming)
		{
			// 注视模式 — 使用锁定速度
			float CurrentSpeed = MoveComp->MaxWalkSpeed;
			if (FMath::IsNearlyEqual(CurrentSpeed, GetWalkSpeed(), 1.0f) || CurrentSpeed <= GetWalkSpeed() + 10.0f)
			{
				MoveComp->MaxWalkSpeed = GetLockedWalkSpeed();
			}
			else
			{
				MoveComp->MaxWalkSpeed = GetLockedRunSpeed();
			}
		}
		else
		{
			// 非注视模式 — 恢复非锁定速度
			float CurrentSpeed = MoveComp->MaxWalkSpeed;
			if (FMath::IsNearlyEqual(CurrentSpeed, GetLockedWalkSpeed(), 1.0f) || CurrentSpeed <= GetLockedWalkSpeed() + 10.0f)
			{
				MoveComp->MaxWalkSpeed = GetWalkSpeed();
			}
			else
			{
				MoveComp->MaxWalkSpeed = GetRunSpeed();
			}
		}
	}
}
