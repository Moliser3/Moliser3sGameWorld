#include "Skill/MeleeSlashSkill.h"
#include "Component/Damage/DamageCalculatorComponent.h"
#include "Component/Attribute/AttributeComponent.h"
#include "Component/Equipment/EquipmentComponent.h"
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
				UAttributeComponent* MyAttrComp = Instigator->FindComponentByClass<UAttributeComponent>();
				if (!MyAttrComp) continue;
				const FCharacterCoreData& MyData = MyAttrComp->GetCharacterData();
				const FCharacterCoreData& TargetData = TargetAttr->GetCharacterData();

				FString AttackerName = Instigator->GetName();
				FString TargetName = Enemy->GetName();

				// 装备加成数据
				UEquipmentComponent* MyEqComp = Instigator->FindComponentByClass<UEquipmentComponent>();
				UEquipmentComponent* TargetEqComp = Enemy->FindComponentByClass<UEquipmentComponent>();
				int32 AtkJin=0, AtkMu=0, AtkShui=0, AtkHuo=0, AtkTu=0;
				int32 DefJin=0, DefMu=0, DefShui=0, DefHuo=0, DefTu=0;
				if (MyEqComp) MyEqComp->GetTotalWuXingBonuses(AtkJin, AtkMu, AtkShui, AtkHuo, AtkTu);
				if (TargetEqComp) TargetEqComp->GetTotalWuXingBonuses(DefJin, DefMu, DefShui, DefHuo, DefTu);

				if (GEngine)
				{
					UE_LOG(LogTemp, Warning, TEXT("=== 伤害 [%s → %s] ==="), *AttackerName, *TargetName);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
						FString::Printf(TEXT("=== 伤害 [%s → %s] ==="), *AttackerName, *TargetName));

					UE_LOG(LogTemp, Warning, TEXT("技能:%s 阶段[%d] 五行:%s 外伤占比:%.0f%%"),
						*SkillName.ToString(), GetCurrentStage(),
						*UEnum::GetValueAsString(Stage.SkillWuXing),
						Stage.ExternalDamageRatio * 100.0f);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White,
						FString::Printf(TEXT("技能:%s 阶段[%d] 五行:%s 外伤占比:%.0f%%"),
							*SkillName.ToString(), GetCurrentStage(),
							*UEnum::GetValueAsString(Stage.SkillWuXing),
							Stage.ExternalDamageRatio * 100.0f));

					UE_LOG(LogTemp, Warning, TEXT("攻击方五行[金%d 木%d 水%d 火%d 土%d] 装备加成[金%d 木%d 水%d 火%d 土%d]"),
						MyData.Jin, MyData.Mu, MyData.Shui, MyData.Huo, MyData.Tu,
						AtkJin, AtkMu, AtkShui, AtkHuo, AtkTu);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
						FString::Printf(TEXT("攻击方五行[金%d 木%d 水%d 火%d 土%d] 装备加成[金%d 木%d 水%d 火%d 土%d]"),
							MyData.Jin, MyData.Mu, MyData.Shui, MyData.Huo, MyData.Tu,
							AtkJin, AtkMu, AtkShui, AtkHuo, AtkTu));

					UE_LOG(LogTemp, Warning, TEXT("防御方五行[金%d 木%d 水%d 火%d 土%d] 装备加成[金%d 木%d 水%d 火%d 土%d] 主属性:%s"),
						TargetData.Jin, TargetData.Mu, TargetData.Shui, TargetData.Huo, TargetData.Tu,
						DefJin, DefMu, DefShui, DefHuo, DefTu,
						*UEnum::GetValueAsString(TargetData.GetDominantWuXing()));
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
						FString::Printf(TEXT("防御方五行[金%d 木%d 水%d 火%d 土%d] 装备加成[金%d 木%d 水%d 火%d 土%d] 主属性:%s"),
							TargetData.Jin, TargetData.Mu, TargetData.Shui, TargetData.Huo, TargetData.Tu,
							DefJin, DefMu, DefShui, DefHuo, DefTu,
							*UEnum::GetValueAsString(TargetData.GetDominantWuXing())));

					UE_LOG(LogTemp, Warning, TEXT("基础伤害:%.1f 五行倍率:%.2f 暴击:%s"),
						Result.RawDamage, Result.WuXingMultiplier, Result.bCrit ? TEXT("是") : TEXT("否"));
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
						FString::Printf(TEXT("基础伤害:%.1f 五行倍率:%.2f 暴击:%s"),
							Result.RawDamage, Result.WuXingMultiplier, Result.bCrit ? TEXT("是") : TEXT("否")));

					float ExtDef = TargetAttr->GetExternalDefense();
					float IntDef = TargetAttr->GetInternalDefense();
					float ExtDefRate = ExtDef / (ExtDef + 100.0f) * 100.0f;
					float IntDefRate = IntDef / (IntDef + 100.0f) * 100.0f;
					UE_LOG(LogTemp, Warning, TEXT("外伤:%.1f(外防%.1f 减免%.1f%% -%.1f) 内伤:%.1f(内防%.1f 减免%.1f%% -%.1f)"),
						Result.ExternalDamage, ExtDef, ExtDefRate, Result.ExternalDefenseReduced,
						Result.InternalDamage, IntDef, IntDefRate, Result.InternalDefenseReduced);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange,
						FString::Printf(TEXT("外伤:%.1f(外防%.1f 减免%.1f%% -%.1f) 内伤:%.1f(内防%.1f 减免%.1f%% -%.1f)"),
							Result.ExternalDamage, ExtDef, ExtDefRate, Result.ExternalDefenseReduced,
							Result.InternalDamage, IntDef, IntDefRate, Result.InternalDefenseReduced));

					UE_LOG(LogTemp, Warning, TEXT(">>> 总伤害: %.0f <<<"), Result.FinalDamage);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
						FString::Printf(TEXT(">>> 总伤害: %.0f <<<"), Result.FinalDamage));
				}

				TargetAttr->TakeDamage(Result.FinalDamage, Instigator);
			}
		}
	}
}
