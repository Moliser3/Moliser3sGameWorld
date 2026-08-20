#include "Component/Damage/DamageCalculatorComponent.h"
#include "Component/Attribute/AttributeComponent.h"
#include "Math/UnrealMathUtility.h"

UDamageCalculatorComponent::UDamageCalculatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FDamageResult UDamageCalculatorComponent::CalculateDamage(
	UAttributeComponent* TargetAttribute,
	float BaseOverride,
	float ExternalDamageRatio) const
{
	FDamageResult Result;

	if (!TargetAttribute)
	{
		return Result;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return Result;
	}

	UAttributeComponent* MyAttribute = Owner->FindComponentByClass<UAttributeComponent>();
	if (!MyAttribute)
	{
		return Result;
	}

	// Step1：基础伤害 = 攻击力 × 技能倍率
	float AttackPower = MyAttribute->GetBaseDamage();
	float SkillMultiplier = (BaseOverride >= 0.0f) ? BaseOverride : 1.0f;
	float BaseDamage = AttackPower * SkillMultiplier;
	Result.RawDamage = BaseDamage;

	// Step2：暴击判定
	float CritValue = FMath::FRand();
	float AfterCrit = BaseDamage;
	if (CritValue <= MyAttribute->GetCritRate())
	{
		Result.bCrit = true;
		AfterCrit = BaseDamage * MyAttribute->GetCritMultiplier();
	}

	// Step3：拆分外伤/内伤
	float RawExternal = AfterCrit * ExternalDamageRatio;
	float RawInternal = AfterCrit * (1.0f - ExternalDamageRatio);

	// Step4：防御减免（百分比）
	static const float DefenseConstant = 100.0f;

	float ExternalDef = TargetAttribute->GetExternalDefense();
	float InternalDef = TargetAttribute->GetInternalDefense();

	float ExternalDefRate = ExternalDef / (ExternalDef + DefenseConstant);
	float InternalDefRate = InternalDef / (InternalDef + DefenseConstant);

	float ExternalAfterDef = RawExternal * (1.0f - ExternalDefRate);
	float InternalAfterDef = RawInternal * (1.0f - InternalDefRate);

	Result.ExternalDefenseReduced = RawExternal * ExternalDefRate;
	Result.InternalDefenseReduced = RawInternal * InternalDefRate;

	// Step5：百分比伤害减免
	float Reduction = TargetAttribute->GetDamageReduction();
	Result.ExternalDamage = ExternalAfterDef * (1.0f - Reduction);
	Result.InternalDamage = InternalAfterDef * (1.0f - Reduction);

	// 最终总伤害（向下取整，最小1）
	float Total = Result.ExternalDamage + Result.InternalDamage;
	Result.FinalDamage = FMath::Max(1.0f, FMath::FloorToFloat(Total));

	return Result;
}
