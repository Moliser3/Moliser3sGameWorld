// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillBase.generated.h"

class UAnimMontage;

/**
 * 技能基类
 * 所有技能继承此类，重写 Execute 实现具体逻辑
 * Duration 控制技能的持续时间，技能系统根据此值判断技能何时释放完成
 * SkillMontage / MontageSlotName 控制技能播放的动画蒙太奇
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API USkillBase : public UObject
{
	GENERATED_BODY()

public:
	/** 执行技能 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void Execute(AActor* Instigator);

	/**
	 * 在施法者身上播放技能蒙太奇
	 * 子类 Execute 中可调用此方法
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void PlaySkillMontage(AActor* Instigator);

	/** 技能名称（用于调试和显示） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FName SkillName;

	/** 技能持续时间（秒），释放后经过此时间才算完成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0"))
	float Duration = 1.0f;

	/** 技能蒙太奇（动画） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> SkillMontage = nullptr;

	/** 蒙太奇槽位名称（如 DefaultSlot、UpperBody 等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Animation")
	FName MontageSlotName = NAME_None;
};