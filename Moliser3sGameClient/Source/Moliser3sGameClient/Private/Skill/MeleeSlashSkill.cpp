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
	if (!Instigator)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar)
	{
		return;
	}

	// 技能释放时停止移动
	if (AController* Ctl = OwnerChar->GetController())
	{
		Ctl->StopMovement();
	}

	// 播放技能蒙太奇（如果已配置）
	PlaySkillMontage(Instigator);
}

void UMeleeSlashSkill::ApplyDamage(AActor* Instigator)
{
	if (!Instigator)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar)
	{
		return;
	}

	FVector Origin = OwnerChar->GetActorLocation();
	FVector Forward = OwnerChar->GetActorForwardVector();

	// SphereOverlap 检测附近所有 Pawn
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Instigator);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	TArray<FOverlapResult> Overlaps;

	Instigator->GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Origin,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams
	);

	float HalfAngleRad = FMath::DegreesToRadians(HalfAngleDeg);

	// 获取攻击者的伤害计算组件
	UDamageCalculatorComponent* DamageCalc = Instigator->FindComponentByClass<UDamageCalculatorComponent>();
	if (!DamageCalc)
	{
		return;
	}

	// 用 TSet 记录已命中的敌人，防止同一个 Actor 被多次伤害
	TSet<AActor*> DamagedActors;

	// 遍历检测结果，过滤扇形内的敌人
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor);
		if (!Enemy)
		{
			continue;
		}

		FVector DirToTarget = (Enemy->GetActorLocation() - Origin).GetSafeNormal2D();
		if (DirToTarget.IsNearlyZero())
		{
			continue;
		}

		// 检查高度差
		float ZDiff = FMath::Abs(Enemy->GetActorLocation().Z - Origin.Z);
		if (ZDiff > MaxZDiff)
		{
			continue;
		}

		// 检查扇形角度
		float Dot = FVector::DotProduct(Forward, DirToTarget);
		float Angle = FMath::Acos(Dot);
		if (Angle <= HalfAngleRad)
		{
			// 记录已命中，防止重复伤害
			DamagedActors.Add(HitActor);

			// 用技能基础伤害走完整伤害计算链
			if (Enemy->GetAttributeComponent())
			{
				FDamageResult Result = DamageCalc->CalculateDamage(Enemy->GetAttributeComponent(), BaseDamage);
				Enemy->GetAttributeComponent()->TakeDamage(Result.FinalDamage, Instigator);
			}
		}
	}
}