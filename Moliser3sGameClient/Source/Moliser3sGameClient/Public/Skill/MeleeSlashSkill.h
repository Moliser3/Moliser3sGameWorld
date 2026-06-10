// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/SkillBase.h"
#include "MeleeSlashSkill.generated.h"

/**
 * 近战斩击技能
 * 对施法者前方扇形范围内的所有敌人造成伤害
 * 所有参数暴露给蓝图，可在蓝图中直接配置
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UMeleeSlashSkill : public USkillBase
{
	GENERATED_BODY()

public:
	virtual void Execute(AActor* Instigator) override;

	/** 扇形检测半径（攻击距离，单位：厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Radius = 100.0f;

	/** 扇形角度（单侧） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float HalfAngleDeg = 22.5f;

	/** 技能基础伤害 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float BaseDamage = 5.0f;

	/** 最大高度差 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float MaxZDiff = 150.0f;
};
