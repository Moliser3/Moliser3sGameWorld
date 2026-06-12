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
 * 跳跃由 SkillSystemComponent 的 Tick 驱动（虚函数 Update），
 * 与引擎渲染帧完全同步，消除 Timer 驱动导致的卡顿。
 *
 * 两阶段设计：
 *   阶段1（0 ~ FlyDuration）：抛物线位移
 *   阶段2（FlyDuration ~ Duration）：落地收尾动画，角色不再移动
 *   技能总时长由 SkillBase::Duration 控制，蓝图可配置
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UJumpSkill : public USkillBase
{
	GENERATED_BODY()

public:
	UJumpSkill();

	virtual void Execute(AActor* Instigator) override;
	virtual void Update(AActor* Instigator, float DeltaTime) override;
	virtual void OnInterrupt(AActor* Instigator) override;
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
	/** 结束跳跃 */
	void EndJump(AActor* Instigator);

	/** 检测目标位置是否可达 */
	bool IsTargetReachable(AActor* Instigator, const FVector& Target) const;

	// ── 运行时状态 ──

	bool bIsJumping = false;
	FVector JumpStartLoc = FVector::ZeroVector;
	FVector JumpTargetLoc = FVector::ZeroVector;
	/** 跳跃进度（0~1），由 Update 每帧更新 */
	float JumpProgress = 0.0f;
};