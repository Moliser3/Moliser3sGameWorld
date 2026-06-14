// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldPlayerController.h"
#include "Component/Input/ClickDetectionComponent.h"
#include "Component/Camera/CameraControllerComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"
#include "Skill/SkillBase.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "Engine/World.h"

AWorldPlayerController::AWorldPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    DefaultMouseCursor = EMouseCursor::Default;

    ClickDetectionComponent = CreateDefaultSubobject<UClickDetectionComponent>(TEXT("ClickDetectionComponent"));
    CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>(TEXT("CameraControllerComponent"));

    PrimaryActorTick.bCanEverTick = true;
}

void AWorldPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bPendingAttack)
    {
        return;
    }

    APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
    if (!MyCharacter)
    {
        bPendingAttack = false;
        return;
    }

    UFacingComponent* FacingComp = MyCharacter->GetFacingComponent();
    AActor* AimTarget = FacingComp ? FacingComp->GetAimTarget() : nullptr;
    if (!AimTarget)
    {
        bPendingAttack = false;
        return;
    }

    float Dist = FVector::Dist(MyCharacter->GetActorLocation(), AimTarget->GetActorLocation());
    bool bInRange = Dist <= PendingMaxRange + 80.0f;
    if (bInRange)
    {
        USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
        if (SkillSys)
        {
            bPendingAttack = false;
            SkillSys->ActivateNextSkill();
        }
    }
}

void AWorldPlayerController::BeginPlay()
{
    Super::BeginPlay();
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    if (DefaultMappingContext)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AWorldPlayerController::OnLeftMouseClick()
{
    // 左键已废弃
}

void AWorldPlayerController::OnRightMouseClick()
{
	if (!ClickDetectionComponent)
	{
		return;
	}

	APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
	if (!MyCharacter)
	{
		return;
	}

	USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
	UFacingComponent* FacingComp = MyCharacter->GetFacingComponent();

	// 执行点击检测
	FClickHitResult ClickResult = ClickDetectionComponent->DetectMouseClick(false);

	// 更新 LastClickTarget
	if (ClickResult.bHitSuccess)
	{
		LastClickTarget = ClickResult.HitLocation;
	}

	// ── 处理点击敌人 ──
	AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get());
	if (ClickedEnemy && SkillSys)
	{
		// 设置注视目标
		if (FacingComp)
			FacingComp->SetAimTarget(ClickedEnemy);
		LastClickTarget = ClickedEnemy->GetActorLocation();

		float MaxRange = SkillSys->GetMaxSkillRange();
		ESkillCategory NextCat = SkillSys->GetNextSkillCategory();

		// 位移/辅助技能 → 直接执行（不受距离约束）
		if (NextCat == ESkillCategory::Movement || NextCat == ESkillCategory::Utility)
		{
			bPendingAttack = false;
			SkillSys->ActivateNextSkill();
			return;
		}

		// 非移动技能：检查距离
		float Dist = FVector::Dist(MyCharacter->GetActorLocation(), ClickedEnemy->GetActorLocation());

		if (MaxRange > 0 && Dist > MaxRange)
		{
			// 超出攻击范围 → 走近后自动攻击
			bPendingAttack = true;
			PendingMaxRange = MaxRange;
			MyCharacter->MoveToLocation(ClickedEnemy->GetActorLocation());
			return;
		}

		// 在攻击范围内 → 直接攻击
		bPendingAttack = false;
		SkillSys->ActivateNextSkill();
		return;
	}

	// ── 点击地面：位移/辅助/复合技能直接执行（跳跃到目标位置或冲锋等）──
	if (ClickResult.bHitSuccess && SkillSys)
	{
		ESkillCategory NextCat = SkillSys->GetNextSkillCategory();
		if (NextCat == ESkillCategory::Movement || NextCat == ESkillCategory::Hybrid || NextCat == ESkillCategory::Utility)
		{
			bPendingAttack = false;
			SkillSys->ActivateNextSkill();
			return;
		}
	}

	// ── 点击地面 → 移动 ──
	if (ClickResult.bHitSuccess)
	{
		if (FacingComp && FacingComp->GetCurrentFacingMode() == EFacingMode::Aiming)
		{
			MyCharacter->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
			MyCharacter->SetAimingFullSpeed(true);
		}
		else if (FacingComp)
		{
			MyCharacter->SetAimingFullSpeed(false);
		}
		MyCharacter->MoveToLocation(ClickResult.HitLocation);
	}
}