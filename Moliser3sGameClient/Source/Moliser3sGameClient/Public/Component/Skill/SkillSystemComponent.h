// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill/SkillTypes.h"
#include "SkillSystemComponent.generated.h"

class USkillBase;

UENUM(BlueprintType)
enum class ESkillPhase : uint8
{
	Idle        UMETA(DisplayName = "空闲"),
	Windup      UMETA(DisplayName = "前摇"),
	Recovery    UMETA(DisplayName = "后摇"),
	LinkWindow  UMETA(DisplayName = "衔接")
};

/**
 * 技能系统组件
 * 管理技能组、技能阶段状态机（前摇→技能触发→后摇→衔接）
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API USkillSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillSystemComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 激活下一个技能（组内循环释放） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ActivateNextSkill();

	/** 添加一个技能到当前技能组 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void AddSkill(USkillBase* NewSkill);

	/** 获取所有技能（当前技能组的全部技能） */
	UFUNCTION(BlueprintPure, Category = "Skill")
	const TArray<USkillBase*>& GetAllSkills() const { return SkillGroup; }

	/** 获取当前技能组的最大释放技能距离（厘米），-1 表示全远程 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	float GetMaxSkillRange() const;

	/** 获取当前正在执行的技能（nullptr 表示空闲） */
	UFUNCTION(BlueprintPure, Category = "Skill")
	USkillBase* GetCurrentSkill() const { return CurrentSkill; }

	/** 获取当前技能阶段 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	ESkillPhase GetSkillPhase() const { return SkillPhase; }

	/** 预览组内下一个技能 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	USkillBase* PeekNextSkill() const;

	/** 获取下一个技能的类别 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	ESkillCategory GetNextSkillCategory() const;

	/** 获取组内技能索引 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	int32 GetGroupSkillIndex() const { return GroupSkillIndex; }

protected:
	/** 当前技能组（未来多组扩展时改为 TArray<FSkillGroupInfo>） */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Skills")
	TArray<TObjectPtr<USkillBase>> SkillGroup;

	/** 当前技能阶段 */
	UPROPERTY(VisibleInstanceOnly, Category = "Skills")
	ESkillPhase SkillPhase = ESkillPhase::Idle;

	/** 当前阶段开始时间戳 */
	float PhaseStartTime = 0.0f;

	/** 当前正在执行的技能（nullptr 表示空闲或衔接等待中） */
	UPROPERTY(VisibleInstanceOnly, Category = "Skills")
	TObjectPtr<USkillBase> CurrentSkill = nullptr;

	/** 技能组内的当前索引 */
	UPROPERTY(VisibleInstanceOnly, Category = "Skills")
	int32 GroupSkillIndex = 0;

	/** 当前技能组索引（预留多组扩展） */
	UPROPERTY(VisibleInstanceOnly, Category = "Skills")
	int32 CurrentGroupIndex = 0;

	/** 缓存衔接时间（当前技能结束时保存 CustomLinkTime，供 LinkWindow 阶段使用） */
	float CachedLinkDuration = 0.0f;
};
