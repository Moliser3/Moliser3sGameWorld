// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Skill/SkillTypes.h"
#include "SkillBase.generated.h"

class UAnimMontage;

/**
 * 技能基类
 * 所有技能继承此类。
 *
 * 技能生命周期：前摇(Windup) → 技能触发(OnExecute) → 后摇(Recovery) → 衔接时间(Link)
 * 前摇不可打断，后摇可被打断，衔接时间决定技能组索引是否重置
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API USkillBase : public UObject
{
	GENERATED_BODY()

public:
	/** 执行技能（前摇开始时调用，播放蒙太奇、初始化状态等） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void Execute(AActor* Instigator);

	/** 前摇每帧更新（跳跃抛物线、蓄力等持续性逻辑） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnWindupUpdate(AActor* Instigator, float DeltaTime);

	/**
	 * 技能触发（前摇→后摇切换帧调用）
	 * 伤害技能在此 ApplyDamage，跳跃在此落地
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnExecute(AActor* Instigator);

	/** 后摇每帧更新 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnRecoveryUpdate(AActor* Instigator, float DeltaTime);

	/** 技能被打断时调用（后摇期间被点击打断），清理运行时状态 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnInterrupt(AActor* Instigator);

	/** 在施法者身上播放技能蒙太奇 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void PlaySkillMontage(AActor* Instigator);

	/** 技能名称（用于调试和显示） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "技能名称"))
	FName SkillName;

	/** 前摇 — 技能起手阶段时长（秒），此阶段不可打断 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", DisplayName = "前摇"))
	float WindupTime = 0.3f;

	/** 后摇 — 技能收尾阶段时长（秒），此阶段可被玩家打断 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", DisplayName = "后摇"))
	float RecoveryTime = 0.5f;

	/** 衔接时间（秒）— 后摇结束后额外等待时长，此时间内点击可连贯到下一技能，超时则重置技能组索引 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", DisplayName = "衔接时间"))
	float CustomLinkTime = 0.2f;

	/** 最大释放技能距离（厘米），-1 表示无限制（远程/位移技能填写），近战技能填具体值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "-1.0", DisplayName = "最大释放技能距离"))
	float MaxSkillRange = 100.0f;

	/** 技能分类（攻击/位移/辅助/复合），决定在控制器中如何响应点击 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "技能分类"))
	ESkillCategory SkillCategory = ESkillCategory::Attack;

	/** 技能蒙太奇（动画） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Animation", meta = (DisplayName = "技能蒙太奇"))
	TObjectPtr<UAnimMontage> SkillMontage = nullptr;

	/** 蒙太奇槽位名称（如 DefaultSlot、UpperBody 等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Animation", meta = (DisplayName = "蒙太奇槽位名称"))
	FName MontageSlotName = NAME_None;
};
