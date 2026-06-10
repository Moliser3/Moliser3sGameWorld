// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Damage/DamageCalculatorComponent.h"
#include "Component/Attribute/AttributeComponent.h"
#include "Math/UnrealMathUtility.h"

UDamageCalculatorComponent::UDamageCalculatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FDamageResult UDamageCalculatorComponent::CalculateDamage(UAttributeComponent* TargetAttribute, float BaseOverride) const
{
	FDamageResult Result;

	if (!TargetAttribute)
	{
		return Result;
	}

	// 获取攻击者的攻击属性（从自身 Owner 的 AttributeComponent 读取）
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

	// Step1：基础伤害（支持外部覆盖）
	float BaseDamage = (BaseOverride >= 0.0f) ? BaseOverride : MyAttribute->GetBaseDamage();
	Result.RawDamage = BaseDamage;

	// Step2：暴击判定
	float CritValue = FMath::FRand();
	if (CritValue <= MyAttribute->GetCritRate())
	{
		Result.bCrit = true;
		BaseDamage *= MyAttribute->GetCritMultiplier();
	}

	// Step3：护甲减免（固定减伤）
	float Armor = TargetAttribute->GetArmor();
	float AfterArmor = FMath::Max(0.0f, BaseDamage - Armor);
	Result.ArmorReduced = FMath::Min(BaseDamage, Armor);

	// Step4：百分比伤害减免
	float Reduction = TargetAttribute->GetDamageReduction();
	float FinalDamage = AfterArmor * (1.0f - Reduction);

	// 最终伤害向下取整，最小为1
	Result.FinalDamage = FMath::Max(1.0f, FMath::FloorToFloat(FinalDamage));

	return Result;
}