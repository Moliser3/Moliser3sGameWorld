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
 *
 * 生命周期：
 *   前摇(Windup) = 抛物线飞行（不可打断）
 *   技能触发(OnExecute) = 落地
 *   后摇(Recovery) = 落地收尾动画（可打断）
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UJumpSkill : public USkillBase
{
	GENERATED_BODY()

public:
	UJumpSkill();

	virtual void Execute(AActor* Instigator) override;
	virtual void OnWindupUpdate(AActor* Instigator, float DeltaTime) override;
	virtual void OnExecute(AActor* Instigator) override;
	virtual void OnRecoveryUpdate(AActor* Instigator, float DeltaTime) override {}
	virtual void OnInterrupt(AActor* Instigator) override;

	/** 最大跳跃距离（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", DisplayName = "最大跳跃距离"))
	float JumpRange = 500.0f;

	/** 抛物线最高点高度（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", DisplayName = "跳跃最高点高度"))
	float JumpHeight = 200.0f;

protected:
	/** 检测目标位置是否可达 */
	bool IsTargetReachable(AActor* Instigator, const FVector& Target) const;

	// ── 运行时状态 ──

	bool bIsJumping = false;
	FVector JumpStartLoc = FVector::ZeroVector;
	FVector JumpTargetLoc = FVector::ZeroVector;
	float JumpProgress = 0.0f;
};
