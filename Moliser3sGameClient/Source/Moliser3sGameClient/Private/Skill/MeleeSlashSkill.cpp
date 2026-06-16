#include "Skill/MeleeSlashSkill.h"
#include "Component/Damage/DamageCalculatorComponent.h"
#include "Component/Attribute/AttributeComponent.h"
#include "EnemyCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

void UMeleeSlashSkill::Execute(AActor* Instigator)
{
	if (!Instigator) return;

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar) return;

	if (AController* Ctl = OwnerChar->GetController())
	{
		Ctl->StopMovement();
	}

	PlaySkillMontage(Instigator);
}

void UMeleeSlashSkill::OnExecute(AActor* Instigator)
{
	ApplyDamage(Instigator);
}

void UMeleeSlashSkill::ApplyDamage(AActor* Instigator)
{
	if (!Instigator || !Stages.IsValidIndex(GetCurrentStage()))
	{
		return;
	}

	const FSkillStage& Stage = Stages[GetCurrentStage()];

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar) return;

	FVector Origin = OwnerChar->GetActorLocation();
	FVector Forward = OwnerChar->GetActorForwardVector();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Instigator);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(MaxSkillRange);
	TArray<FOverlapResult> Overlaps;

	Instigator->GetWorld()->OverlapMultiByChannel(
		Overlaps, Origin, FQuat::Identity, ECC_Pawn, Sphere, QueryParams
	);

	float HalfAngleRad = FMath::DegreesToRadians(Stage.HalfAngleDeg);

	UDamageCalculatorComponent* DamageCalc = Instigator->FindComponentByClass<UDamageCalculatorComponent>();
	if (!DamageCalc) return;

	TSet<AActor*> DamagedActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || DamagedActors.Contains(HitActor)) continue;

		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor);
		if (!Enemy) continue;

		FVector DirToTarget = (Enemy->GetActorLocation() - Origin).GetSafeNormal2D();
		if (DirToTarget.IsNearlyZero()) continue;

		float ZDiff = FMath::Abs(Enemy->GetActorLocation().Z - Origin.Z);
		if (ZDiff > Stage.MaxZDiff) continue;

		float Dot = FVector::DotProduct(Forward, DirToTarget);
		float Angle = FMath::Acos(Dot);
		if (Angle <= HalfAngleRad)
		{
			DamagedActors.Add(HitActor);

			if (Enemy->GetAttributeComponent())
			{
				FDamageResult Result = DamageCalc->CalculateDamage(Enemy->GetAttributeComponent(), Stage.BaseDamage);
				Enemy->GetAttributeComponent()->TakeDamage(Result.FinalDamage, Instigator);
			}
		}
	}
}
