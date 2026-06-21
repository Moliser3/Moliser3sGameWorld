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
	ESkillWuXing SkillWuXing,
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

	// Step2：五行相克计算
	EWuXing TargetWuXing = TargetAttribute->GetCharacterData().GetDominantWuXing();
	float WuXingMult = GetWuXingMultiplier(SkillWuXing, TargetWuXing);
	Result.WuXingMultiplier = WuXingMult;
	float AfterWuXing = BaseDamage * WuXingMult;

	// Step3：暴击判定
	float CritValue = FMath::FRand();
	if (CritValue <= MyAttribute->GetCritRate())
	{
		Result.bCrit = true;
		AfterWuXing *= MyAttribute->GetCritMultiplier();
	}

	// Step4：拆分外伤/内伤
	float RawExternal = AfterWuXing * ExternalDamageRatio;
	float RawInternal = AfterWuXing * (1.0f - ExternalDamageRatio);

	// Step5：防御减免
	float ExternalDef = TargetAttribute->GetExternalDefense();
	float InternalDef = TargetAttribute->GetInternalDefense();

	float ExternalAfterDef = FMath::Max(0.0f, RawExternal - ExternalDef);
	float InternalAfterDef = FMath::Max(0.0f, RawInternal - InternalDef);

	Result.ExternalDefenseReduced = FMath::Min(RawExternal, ExternalDef);
	Result.InternalDefenseReduced = FMath::Min(RawInternal, InternalDef);

	// Step6：百分比伤害减免
	float Reduction = TargetAttribute->GetDamageReduction();
	Result.ExternalDamage = ExternalAfterDef * (1.0f - Reduction);
	Result.InternalDamage = InternalAfterDef * (1.0f - Reduction);

	// 最终总伤害（向下取整，最小1）
	float Total = Result.ExternalDamage + Result.InternalDamage;
	Result.FinalDamage = FMath::Max(1.0f, FMath::FloorToFloat(Total));

	return Result;
}

float UDamageCalculatorComponent::GetWuXingMultiplier(ESkillWuXing AttackWuXing, EWuXing DefenseWuXing)
{
	// 相克表 [攻方][守方]: true = 攻方克守方
	static const bool KeTable[5][5] = {
		// Jin    Mu     Shui   Huo    Tu
		{ false, true,  false, false, false },  // Jin 克 Mu
		{ false, false, false, false, true  },  // Mu 克 Tu
		{ false, false, false, true,  false },  // Shui 克 Huo
		{ true,  false, false, false, false },  // Huo 克 Jin
		{ false, false, true,  false, false },  // Tu 克 Shui
	};

	int32 AttackIdx = static_cast<int32>(AttackWuXing);
	int32 DefenseIdx = static_cast<int32>(DefenseWuXing);

	if (AttackIdx < 0 || AttackIdx > 4 || DefenseIdx < 0 || DefenseIdx > 4)
	{
		return 1.0f;
	}

	if (KeTable[AttackIdx][DefenseIdx])
	{
		return 1.3f;  // 攻方克守方
	}
	if (KeTable[DefenseIdx][AttackIdx])
	{
		return 0.7f;  // 守方克攻方
	}
	return 1.0f;  // 无关
}
