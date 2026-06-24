#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"
#include "Component/Inventory/QuickSlotComponent.h"
#include "WorldPlayerController.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	FacingComponent = CreateDefaultSubobject<UFacingComponent>(TEXT("FacingComponent"));
	SkillSystemComponent = CreateDefaultSubobject<USkillSystemComponent>(TEXT("SkillSystemComponent"));
	QuickSlotComponent = CreateDefaultSubobject<UQuickSlotComponent>(TEXT("QuickSlotComponent"));
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->MaxWalkSpeed = GetRunSpeed();
		MoveComp->JumpZVelocity = 420.0f;
		MoveComp->AirControl = 0.3f;
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AWorldPlayerController* PC = Cast<AWorldPlayerController>(GetController()))
	{
		PC->OnCombatStateChanged.AddDynamic(this, &APlayerCharacter::OnCombatStateChanged);
		PC->OnActionStateChanged.AddDynamic(this, &APlayerCharacter::OnActionStateChanged);
	}

	// ============================================================
	// 【Debug 快捷栏测试 — 上线前需删除】
	// ============================================================
	for (int32 i = 0; i < TestQuickSlotItems.Num() && i < QuickSlotComponent->SlotCount; i++)
	{
		if (TestQuickSlotItems[i])
		{
			QuickSlotComponent->AssignSlot(i, TestQuickSlotItems[i]);
		}
	}
}

void APlayerCharacter::OnCombatStateChanged(ECombatState NewCombat)
{
	if (NewCombat == ECombatState::BattlePerception)
	{
		FacingComponent->SetMode(EFacingMode::Aiming);
	}
}

void APlayerCharacter::OnActionStateChanged(EActionState NewAction)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	switch (NewAction)
	{
	case EActionState::Idle:
		MoveComp->MaxWalkSpeed = 0.0f;
		break;

	case EActionState::Walking:
		MoveComp->MaxWalkSpeed = GetWalkSpeed();
		break;

	case EActionState::Running:
		MoveComp->MaxWalkSpeed = GetRunSpeed();
		break;

	case EActionState::Skill:
		if (AController* Ctl = GetController())
		{
			Ctl->StopMovement();
		}
		break;
	}
}
