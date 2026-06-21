#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill/SkillTypes.h"
#include "Data/DataDefinitions.h"
#include "DamageCalculatorComponent.generated.h"

class UAttributeComponent;

/** 伤害结果结构体 */
USTRUCT(BlueprintType)
struct FDamageResult
{
	GENERATED_BODY()

	/** 原始伤害（暴击/五行修正前） */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float RawDamage = 0.0f;

	/** 最终总伤害 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float FinalDamage = 0.0f;

	/** 是否暴击 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bCrit = false;

	/** 五行相克倍率 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float WuXingMultiplier = 1.0f;

	/** 外伤部分 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float ExternalDamage = 0.0f;

	/** 内伤部分 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float InternalDamage = 0.0f;

	/** 护甲减免量 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float ExternalDefenseReduced = 0.0f;

	/** 内抗减免量 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float InternalDefenseReduced = 0.0f;
};

/**
 * 伤害计算组件
 * 挂在攻击者身上，负责计算五行生克 + 外伤内伤 + 暴击 + 防御减免
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UDamageCalculatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageCalculatorComponent();

	/**
	 * 计算最终伤害
	 * @param TargetAttribute 目标的属性组件
	 * @param BaseOverride 覆盖基础伤害值（技能倍率），小于0时使用自身攻击力
	 * @param SkillWuXing 技能五行属性（用于生克计算）
	 * @param ExternalDamageRatio 外伤占比(0~1)，剩余为内伤
	 * @return 伤害计算结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	FDamageResult CalculateDamage(
		UAttributeComponent* TargetAttribute,
		float BaseOverride = -1.0f,
		ESkillWuXing SkillWuXing = ESkillWuXing::Jin,
		float ExternalDamageRatio = 0.7f
	) const;

	/** 五行相克倍率查表 */
	static float GetWuXingMultiplier(ESkillWuXing AttackWuXing, EWuXing DefenseWuXing);
};
