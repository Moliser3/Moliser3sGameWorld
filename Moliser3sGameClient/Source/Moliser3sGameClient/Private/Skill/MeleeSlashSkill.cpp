#include "Skill/MeleeSlashSkill.h"
#include "Component/Damage/DamageCalculatorComponent.h"
#include "Component/Attribute/AttributeComponent.h"
#include "EnemyCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "DebugHelper.h"

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
				UAttributeComponent* TargetAttr = Enemy->GetAttributeComponent();
				FDamageResult Result = DamageCalc->CalculateDamage(
					TargetAttr,
					Stage.BaseDamage,
					Stage.SkillWuXing,
					Stage.ExternalDamageRatio
				);

				// 伤害分解日志
				const FCharacterCoreData& MyData = Instigator->FindComponentByClass<UAttributeComponent>()->GetCharacterData();
				const FCharacterCoreData& TargetData = TargetAttr->GetCharacterData();

				FString AttackerName = Instigator->GetName();
				FString TargetName = Enemy->GetName();

				if (GEngine)
				{
					UE_LOG(LogTemp, Warning, TEXT("=== Damage [%s -> %s] ==="), *AttackerName, *TargetName);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
						FString::Printf(TEXT("=== Damage [%s -> %s] ==="), *AttackerName, *TargetName));

					UE_LOG(LogTemp, Warning, TEXT("Skill:%s Stage[%d] Wuxing:%s ExtRatio:%.0f%%"),
						*SkillName.ToString(), GetCurrentStage(),
						*UEnum::GetValueAsString(Stage.SkillWuXing),
						Stage.ExternalDamageRatio * 100.0f);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White,
						FString::Printf(TEXT("Skill:%s Stage[%d] Wuxing:%s ExtRatio:%.0f%%"),
							*SkillName.ToString(), GetCurrentStage(),
							*UEnum::GetValueAsString(Stage.SkillWuXing),
							Stage.ExternalDamageRatio * 100.0f));

					UE_LOG(LogTemp, Warning, TEXT("Atk Five: %d/%d/%d/%d/%d"),
						MyData.Jin, MyData.Mu, MyData.Shui, MyData.Huo, MyData.Tu);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
						FString::Printf(TEXT("Atk Five: %d/%d/%d/%d/%d"), MyData.Jin, MyData.Mu, MyData.Shui, MyData.Huo, MyData.Tu));

					UE_LOG(LogTemp, Warning, TEXT("Def Five: %d/%d/%d/%d/%d (Dom:%s)"),
						TargetData.Jin, TargetData.Mu, TargetData.Shui, TargetData.Huo, TargetData.Tu,
						*UEnum::GetValueAsString(TargetData.GetDominantWuXing()));
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
						FString::Printf(TEXT("Def Five: %d/%d/%d/%d/%d (Dom:%s)"),
							TargetData.Jin, TargetData.Mu, TargetData.Shui, TargetData.Huo, TargetData.Tu,
							*UEnum::GetValueAsString(TargetData.GetDominantWuXing())));

					UE_LOG(LogTemp, Warning, TEXT("Raw:%.1f Wuxing:%.2f Crit:%s"),
						Result.RawDamage, Result.WuXingMultiplier, Result.bCrit ? TEXT("Y") : TEXT("N"));
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
						FString::Printf(TEXT("Raw:%.1f Wuxing:%.2f Crit:%s"),
							Result.RawDamage, Result.WuXingMultiplier, Result.bCrit ? TEXT("Y") : TEXT("N")));

					UE_LOG(LogTemp, Warning, TEXT("Ext:%.1f(Def-%.1f) Int:%.1f(Def-%.1f)"),
						Result.ExternalDamage, Result.ExternalDefenseReduced,
						Result.InternalDamage, Result.InternalDefenseReduced);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange,
						FString::Printf(TEXT("Ext:%.1f(Def-%.1f) Int:%.1f(Def-%.1f)"),
							Result.ExternalDamage, Result.ExternalDefenseReduced,
							Result.InternalDamage, Result.InternalDefenseReduced));

					UE_LOG(LogTemp, Warning, TEXT(">>> Total Damage: %.0f <<<"), Result.FinalDamage);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
						FString::Printf(TEXT(">>> Total Damage: %.0f <<<"), Result.FinalDamage));
				}

				TargetAttr->TakeDamage(Result.FinalDamage, Instigator);
			}
		}
	}
}
