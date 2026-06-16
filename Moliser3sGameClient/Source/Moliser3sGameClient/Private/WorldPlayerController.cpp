#include "WorldPlayerController.h"
#include "Component/Input/ClickDetectionComponent.h"
#include "Component/Camera/CameraControllerComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"
#include "Skill/SkillBase.h"
#include "Animation/AnimInstance.h"
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

	APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
	if (!MyCharacter) return;

	UFacingComponent* FacingComp = MyCharacter->GetFacingComponent();
	USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
	if (!FacingComp || !SkillSys) return;

	AActor* AimTarget = FacingComp->GetAimTarget();

	// ── 1. 战斗状态转移：超出战斗感知范围或无目标 → 默认状态 ──
	if (CurrentPlayerState == EPlayerState::Battle)
	{
		if (!AimTarget)
		{
			SetPlayerState(EPlayerState::Default);
		}
		else
		{
			float Dist = FVector::Dist(MyCharacter->GetActorLocation(), AimTarget->GetActorLocation());
			if (Dist > MyCharacter->GetBattlePerceptionRange())
			{
				SetPlayerState(EPlayerState::Default);
			}
		}
	}

	// ── 2. 技能结束恢复注视 ──
	ESkillPhase CurrentPhase = SkillSys->GetSkillPhase();
	if (CurrentPlayerState == EPlayerState::Battle && bPreviousSkillActive && CurrentPhase == ESkillPhase::Idle)
	{
		if (UAnimInstance* AnimInst = MyCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_Stop(0.1f);
		}
		AActor* SkillEndTarget = FacingComp->GetAimTarget();
		if (SkillEndTarget && FacingComp->GetCurrentFacingMode() != EFacingMode::Aiming)
		{
			float Dist = FVector::Dist(MyCharacter->GetActorLocation(), SkillEndTarget->GetActorLocation());
			if (Dist <= MyCharacter->GetBattlePerceptionRange())
			{
				FacingComp->SetAimTarget(SkillEndTarget);
			}
		}
	}
	bPreviousSkillActive = (CurrentPhase != ESkillPhase::Idle);

	// ── 3. Shift奔跑结束后的注视恢复 ──
	if (CurrentPlayerState == EPlayerState::Battle && bPendingRestoreAiming)
	{
		AActor* RunTarget = FacingComp->GetAimTarget();
		if (!RunTarget)
		{
			bPendingRestoreAiming = false;
		}
		else if (MyCharacter->GetVelocity().IsNearlyZero())
		{
			bPendingRestoreAiming = false;
			float Dist = FVector::Dist(MyCharacter->GetActorLocation(), RunTarget->GetActorLocation());
			if (Dist <= MyCharacter->GetBattlePerceptionRange())
			{
				if (FacingComp->GetCurrentFacingMode() != EFacingMode::Aiming)
				{
					FacingComp->SetAimTarget(RunTarget);
				}
			}
		}
	}

	// ── 4. 自动攻击 ──
	if (!bPendingAttack) return;

	if (!AimTarget)
	{
		bPendingAttack = false;
		return;
	}

	float Dist = FVector::Dist(MyCharacter->GetActorLocation(), AimTarget->GetActorLocation());
	if (Dist <= PendingMaxRange + 80.0f)
	{
		bPendingAttack = false;
		SkillSys->ActivateLeft();
	}
}

void AWorldPlayerController::SetPlayerState(EPlayerState NewState)
{
	if (CurrentPlayerState == NewState) return;

	APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
	EPlayerState OldState = CurrentPlayerState;
	CurrentPlayerState = NewState;

	if (NewState == EPlayerState::Default)
	{
		bPendingAttack = false;
		bPendingRestoreAiming = false;
		if (MyCharacter)
		{
			if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
			{
				FacingComp->ClearAimTarget();
			}
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
	if (!ClickDetectionComponent) return;

	APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
	if (!MyCharacter) return;

	USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
	UFacingComponent* FacingComp = MyCharacter->GetFacingComponent();
	UCharacterMovementComponent* MoveComp = MyCharacter->GetCharacterMovement();
	if (!SkillSys || !FacingComp || !MoveComp) return;

	bool bShiftDown = IsInputKeyDown(EKeys::LeftShift);

	FClickHitResult ClickResult = ClickDetectionComponent->DetectMouseClick(false);

	if (ClickResult.bHitSuccess)
	{
		LastClickTarget = ClickResult.HitLocation;
	}

	// ── 点击敌人 ──
	AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get());
	if (ClickedEnemy)
	{
		FacingComp->SetAimTarget(ClickedEnemy);
		SetPlayerState(EPlayerState::Battle);
		LastClickTarget = ClickedEnemy->GetActorLocation();

		MoveComp->MaxWalkSpeed = bShiftDown ? MyCharacter->GetRunSpeed() : MyCharacter->GetWalkSpeed();

		float MaxRange = SkillSys->GetMaxSkillRange();
		float Dist = FVector::Dist(MyCharacter->GetActorLocation(), ClickedEnemy->GetActorLocation());

		if (MaxRange > 0 && Dist > MaxRange)
		{
			bPendingAttack = true;
			PendingMaxRange = MaxRange;
			MyCharacter->MoveToLocation(ClickedEnemy->GetActorLocation());
			return;
		}

		bPendingAttack = false;
		SkillSys->ActivateLeft();
		return;
	}

	// ── 点击地面 → 移动 ──
	if (ClickResult.bHitSuccess)
	{
		if (CurrentPlayerState == EPlayerState::Battle)
		{
			if (bShiftDown)
			{
				MoveComp->bOrientRotationToMovement = true;
				MoveComp->MaxWalkSpeed = MyCharacter->GetRunSpeed();
				bPendingRestoreAiming = true;
			}
			else
			{
				MoveComp->MaxWalkSpeed = MyCharacter->GetWalkSpeed();
			}
		}
		else
		{
			MoveComp->bOrientRotationToMovement = true;
			MoveComp->MaxWalkSpeed = bShiftDown ? MyCharacter->GetRunSpeed() : MyCharacter->GetWalkSpeed();
		}

		MyCharacter->MoveToLocation(ClickResult.HitLocation);
	}
}

void AWorldPlayerController::OnRightMouseClick()
{
	if (!ClickDetectionComponent) return;

	APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
	if (!MyCharacter) return;

	USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
	if (!SkillSys) return;

	FClickHitResult ClickResult = ClickDetectionComponent->DetectMouseClick(false);

	if (ClickResult.bHitSuccess)
	{
		LastClickTarget = ClickResult.HitLocation;
	}

	// ── 点击敌人 → 进入战斗状态，立即释放右键技能 ──
	AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get());
	if (ClickedEnemy)
	{
		if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
		{
			FacingComp->SetAimTarget(ClickedEnemy);
		}
		SetPlayerState(EPlayerState::Battle);
		LastClickTarget = ClickedEnemy->GetActorLocation();
	}

	// 释放右键技能（不检查距离，直接释放）
	SkillSys->ActivateRight();
}

void AWorldPlayerController::OnAltPressed()
{
	bDefending = true;
}

void AWorldPlayerController::OnAltReleased()
{
	bDefending = false;
}
