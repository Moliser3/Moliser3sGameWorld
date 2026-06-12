// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillBase.generated.h"

class UAnimMontage;

/**
 * 技能基类
 * 所有技能继承此类，重写 Execute 和 ApplyDamage 实现具体逻辑
 *
 * Duration: 技能持续总时长
 * DamageAt: 伤害触发的时间点（动画的"命中帧"）
 * InterruptibleAt: 可打断时间点，超过此点可以提前释放下一个技能
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API USkillBase : public UObject
{
	GENERATED_BODY()

public:
	/** 执行技能（播放蒙太奇、设置参数等） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void Execute(AActor* Instigator);

	/**
	 * 每帧更新技能（由 SkillSystemComponent::TickComponent 调用）
	 * 子类在此方法中实现持续性技能逻辑（如跳跃抛物线更新）
	 * 基类空实现
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void Update(AActor* Instigator, float DeltaTime);

	/**
	 * 技能被打断时调用（由 SkillSystemComponent 在打断时调用）
	 * 子类在此方法中清理自己的运行时状态（如跳跃的 bIsJumping）
	 * 基类空实现
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnInterrupt(AActor* Instigator);

	/**
	 * 延迟应用伤害
	 * 在 DamageAt 时间点由 SkillSystemComponent 调用
	 * 子类在此方法中实现伤害逻辑
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void ApplyDamage(AActor* Instigator);

	/**
	 * 在施法者身上播放技能蒙太奇
	 * 子类 Execute 中可调用此方法
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void PlaySkillMontage(AActor* Instigator);

	/**
	 * 获取实际可打断时间点。
	 * 默认为 InterruptibleAt 属性，子类可重写。
	 * 例如跳跃技能返回 Max(InterruptibleAt, FlyDuration) 确保飞行中不可打断。
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual float GetInterruptibleAt() const { return InterruptibleAt; }

	/** 技能名称（用于调试和显示） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FName SkillName;

	/** 技能持续时间（秒），释放后经过此时间才算完成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0"))
	float Duration = 1.0f;

	/** 伤害触发时间（秒），从技能开始到实际造成伤害的延迟 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0"))
	float DamageAt = 0.3f;

	/** 可打断时间（秒），超过此时间后可以提前释放下一个技能；0=全程可打断 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0"))
	float InterruptibleAt = 0.3f;

	/** 最大攻击距离（厘米），-1 表示无限制（远程技能），近战技能填具体值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "-1.0"))
	float MaxAttackRange = 100.0f;

	/** 是否为移动技能（如跳跃），移动技能不受攻击距离判断约束，点击即可触发 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	bool bIsMovementSkill = false;

	/** 技能蒙太奇（动画） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> SkillMontage = nullptr;

	/** 蒙太奇槽位名称（如 DefaultSlot、UpperBody 等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Animation")
	FName MontageSlotName = NAME_None;
};