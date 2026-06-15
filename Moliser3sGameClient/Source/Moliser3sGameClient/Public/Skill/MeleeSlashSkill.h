// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/DamageSkillBase.h"
#include "MeleeSlashSkill.generated.h"

/**
 * 近战斩击技能
 * 对施法者前方扇形范围内的所有敌人造成伤害
 * 所有参数暴露给蓝图，可在蓝图中直接配置
 *
 * 生命周期：前摇→技能触发(ApplyDamage)→后摇
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UMeleeSlashSkill : public UDamageSkillBase
{
	GENERATED_BODY()

public:
	virtual void Execute(AActor* Instigator) override;

	/** 激发瞬间在前摇→后摇切换帧触发扇形范围伤害 */
	virtual void OnExecute(AActor* Instigator) override;

	virtual void ApplyDamage(AActor* Instigator) override;

	/** 扇形角度（单侧） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "扇形半角"))
	float HalfAngleDeg = 22.5f;

	/** 技能基础伤害 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "技能基础伤害"))
	float BaseDamage = 5.0f;

	/** 最大高度差 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "最大高度差"))
	float MaxZDiff = 150.0f;
};
