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

	// ── 1. 战斗感知状态转移 ──
	if (CurrentCombatState == ECombatState::BattlePerception)
	{
		if (!AimTarget)
		{
			SetCombatState(ECombatState::Default);
		}
		else
		{
			float Dist = FVector::Dist(MyCharacter->GetActorLocation(), AimTarget->GetActorLocation());
			if (Dist > MyCharacter->GetBattlePerceptionRange())
			{
				SetCombatState(ECombatState::Default);
			}
		}
	}

	// ── 2. 技能结束 → 闲置 ──
	ESkillPhase CurrentPhase = SkillSys->GetSkillPhase();
	static bool bPrevSkillActive = false;
	if (CurrentActionState == EActionState::Skill && bPrevSkillActive && CurrentPhase == ESkillPhase::Idle)
	{
		SetActionState(EActionState::Idle);
	}
	bPrevSkillActive = (CurrentPhase != ESkillPhase::Idle);

	// ── 3. 奔跑结束 → 闲置 ──
	if (CurrentActionState == EActionState::Running &&
		GetWorld()->GetTimeSeconds() - RunStartTime > 0.3f &&
		MyCharacter->GetVelocity().SizeSquared() < 100.0f)
	{
		SetActionState(EActionState::Idle);
	}

	// ── 4. 闲置时恢复注视模式 ──
	if (CurrentActionState == EActionState::Idle &&
		CurrentCombatState == ECombatState::BattlePerception &&
		FacingComp->GetAimTarget() &&
		FacingComp->GetCurrentFacingMode() != EFacingMode::Aiming)
	{
		FacingComp->SetMode(EFacingMode::Aiming);
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
		SetActionState(EActionState::Skill);
		SkillSys->ActivateLeft();
	}
}

void AWorldPlayerController::SetCombatState(ECombatState NewState)
{
	if (CurrentCombatState == NewState) return;

	CurrentCombatState = NewState;
	OnCombatStateChanged.Broadcast(NewState);

	if (NewState == ECombatState::Default)
	{
		bPendingAttack = false;
		if (APlayerCharacter* MyChar = Cast<APlayerCharacter>(GetPawn()))
		{
			if (UFacingComponent* FacingComp = MyChar->GetFacingComponent())
			{
				FacingComp->ClearAimTarget();
			}
		}
	}
}

void AWorldPlayerController::SetActionState(EActionState NewState)
{
	if (CurrentActionState == NewState) return;

	CurrentActionState = NewState;
	OnActionStateChanged.Broadcast(NewState);
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
		SetCombatState(ECombatState::BattlePerception);
		LastClickTarget = ClickedEnemy->GetActorLocation();

		MoveComp->MaxWalkSpeed = bShiftDown ? MyCharacter->GetRunSpeed() : MyCharacter->GetWalkSpeed();

		float MaxRange = SkillSys->GetMaxSkillRange();
		float Dist = FVector::Dist(MyCharacter->GetActorLocation(), ClickedEnemy->GetActorLocation());

		if (MaxRange > 0 && Dist > MaxRange)
		{
			bPendingAttack = true;
			PendingMaxRange = MaxRange;
			SetActionState(EActionState::Walking);
			MyCharacter->MoveToLocation(ClickedEnemy->GetActorLocation());
			return;
		}

		bPendingAttack = false;
		SkillSys->ActivateLeft();
		SetActionState(EActionState::Skill);
		return;
	}

	// ── 点击地面 → 移动 ──
	if (ClickResult.bHitSuccess)
	{
		EActionState NewAction = EActionState::Walking;
		if (bShiftDown)
		{
			NewAction = EActionState::Running;
			if (FacingComp->GetCurrentFacingMode() == EFacingMode::Aiming)
			{
				FacingComp->SetMode(EFacingMode::Walking);
			}
			RunStartTime = GetWorld()->GetTimeSeconds();
		}
		else if (CurrentCombatState == ECombatState::BattlePerception)
		{
			if (FacingComp->GetAimTarget() && FacingComp->GetCurrentFacingMode() != EFacingMode::Aiming)
			{
				FacingComp->SetMode(EFacingMode::Aiming);
			}
		}
		SetActionState(NewAction);
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

	AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get());
	if (ClickedEnemy)
	{
		if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
		{
			FacingComp->SetAimTarget(ClickedEnemy);
		}
		SetCombatState(ECombatState::BattlePerception);
		LastClickTarget = ClickedEnemy->GetActorLocation();
	}

	SetActionState(EActionState::Skill);
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
