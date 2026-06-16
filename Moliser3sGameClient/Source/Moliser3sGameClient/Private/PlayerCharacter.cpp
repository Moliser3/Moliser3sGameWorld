#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	FacingComponent = CreateDefaultSubobject<UFacingComponent>(TEXT("FacingComponent"));
	SkillSystemComponent = CreateDefaultSubobject<USkillSystemComponent>(TEXT("SkillSystemComponent"));
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

	if (FacingComponent)
	{
		FacingComponent->OnFacingModeChanged.AddDynamic(this, &APlayerCharacter::UpdateMovementSpeed);
	}
}

void APlayerCharacter::UpdateMovementSpeed(EFacingMode NewMode)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = GetWalkSpeed();
	}
}
