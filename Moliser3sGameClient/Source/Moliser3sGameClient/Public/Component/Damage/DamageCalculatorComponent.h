// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageCalculatorComponent.generated.h"

class UAttributeComponent;

/**
 * 伤害结果结构体
 */
USTRUCT(BlueprintType)
struct FDamageResult
{
	GENERATED_BODY()

	/** 原始伤害 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float RawDamage = 0.0f;

	/** 最终伤害（经过计算后） */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float FinalDamage = 0.0f;

	/** 是否暴击 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bCrit = false;

	/** 护甲减免 */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float ArmorReduced = 0.0f;
};

/**
 * 伤害计算组件
 * 挂在攻击者身上，负责计算最终伤害
 * 读取自己的攻击属性（攻击者），读取目标的防御属性（受击者）
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UDamageCalculatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageCalculatorComponent();

	/**
	 * 计算最终伤害
	 * @param TargetAttribute 目标的属性组件（用于读取防御属性）
	 * @param BaseOverride 可选：覆盖基础伤害值，小于0时使用自身 BaseDamage
	 * @return 伤害计算结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	FDamageResult CalculateDamage(UAttributeComponent* TargetAttribute, float BaseOverride = -1.0f) const;
};