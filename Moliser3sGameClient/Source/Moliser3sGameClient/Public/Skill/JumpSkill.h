// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/SkillBase.h"
#include "JumpSkill.generated.h"

/**
 * 跳跃技能
 * 读取 WorldPlayerController::LastClickTarget 作为目标位置，
 * 沿抛物线从当前位置飞向目标。
 * 目标不可达时原地起跳。
 * 飞行中碰到障碍物则落地。
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UJumpSkill : public USkillBase
{
	GENERATED_BODY()

public:
	virtual void Execute(AActor* Instigator) override;
	virtual void ApplyDamage(AActor* Instigator) override {}

	/** 最大跳跃距离（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0"))
	float JumpRange = 500.0f;

	/** 抛物线最高点高度（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0"))
	float JumpHeight = 200.0f;

	/** 空中飞行时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0"))
	float FlyDuration = 0.6f;

protected:
	/** 每帧更新抛物线位置 */
	void OnJumpTick(AActor* Instigator);

	/** 结束跳跃 */
	void EndJump(AActor* Instigator);

	/** 检测目标位置是否可达 */
	bool IsTargetReachable(AActor* Instigator, const FVector& Target) const;

	// ── 运行时状态 ──

	bool bIsJumping = false;
	FVector JumpStartLoc = FVector::ZeroVector;
	FVector JumpTargetLoc = FVector::ZeroVector;
	float JumpProgress = 0.0f;
	FTimerHandle JumpTimerHandle;
};