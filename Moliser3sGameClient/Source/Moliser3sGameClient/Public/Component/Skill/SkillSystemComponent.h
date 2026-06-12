// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillSystemComponent.generated.h"

class USkillBase;

/**
 * 技能系统组件
 * 挂载在角色上，管理技能列表并提供释放接口
 * 支持技能循环队列：每次调用 ActivateNextSkill 会依次释放队列中的技能，
 * 只有等当前技能的 Duration 结束后，才能再次调用 ActivateNextSkill 释放下一个。
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API USkillSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillSystemComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 获取所有已注册的技能列表（编辑器中配置的完整列表） */
	UFUNCTION(BlueprintPure, Category = "Skill")
	const TArray<USkillBase*>& GetAllSkills() const { return SkillList; }

	/** 获取技能循环队列（运行时循环读取） */
	UFUNCTION(BlueprintPure, Category = "Skill")
	const TArray<USkillBase*>& GetSkillQueue() const { return SkillQueue; }

	/** 设置技能循环队列（由蓝图在 BeginPlay 时调用） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void SetSkillQueue(const TArray<USkillBase*>& InQueue);

	/** 激活下一个技能（按循环队列顺序） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ActivateNextSkill();

	/** 添加一个技能到 SkillList */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void AddSkill(USkillBase* NewSkill);

	/** 当前是否正在释放技能（未完成） */
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsSkillActive() const { return bSkillActive; }

	/** 获取技能队列中第一个近战技能的最大攻击距离（厘米），-1 表示全远程 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	float GetMaxAttackRange() const;

	/** 获取当前正在执行的技能（nullptr 表示空闲） */
	UFUNCTION(BlueprintPure, Category = "Skill")
	USkillBase* GetCurrentSkill() const { return CurrentSkill; }

	/** 预览队列中的下一个技能（不执行），用于判断技能类型 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	USkillBase* PeekNextSkill() const;

	/** 下一个技能是否为移动技能（如跳跃） */
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsNextSkillMovement() const;

	/** 获取当前技能已运行的时间（秒），用于判断是否超过 InterruptibleAt */
	UFUNCTION(BlueprintPure, Category = "Skill")
	float GetCurrentSkillElapsed() const;

	/** 尝试打断当前技能（如果 elapsed ≥ InterruptibleAt），不执行下一个技能 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void TryInterruptCurrentSkill();

	/** 强制结束当前技能，进入连招窗口（供跳跃等自行管理结束时机的技能使用） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ForceEndCurrentSkill();

	/** 连招窗口持续时间（秒），默认可在蓝图中配置 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combo", meta = (ClampMin = "0.0"))
	float ComboWindowDuration = 0.5f;

protected:
	/** 从编辑器中配置的所有技能（蓝图可直接在此数组中配置实例） */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Skills")
	TArray<TObjectPtr<USkillBase>> SkillList;

	/** 技能循环队列 — 运行时按此顺序依次释放（可在蓝图 BeginPlay 中设置） */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Skills")
	TArray<TObjectPtr<USkillBase>> SkillQueue;

	/** 技能循环队列中的当前索引 */
	UPROPERTY(VisibleInstanceOnly, Category = "Skills")
	int32 QueueIndex = 0;

	/** 当前正在释放的技能（nullptr 表示空闲） */
	UPROPERTY(VisibleInstanceOnly, Category = "Skills")
	TObjectPtr<USkillBase> CurrentSkill = nullptr;

	/** 当前技能的开始时间戳 */
	float CurrentSkillStartTime = 0.0f;

	/** 技能是否正处于激活状态（Duration 倒计时中） */
	bool bSkillActive = false;

	/** 是否处于连招缓冲期 */
	bool bInComboWindow = false;

	/** 连招窗口结束时间戳 */
	float ComboWindowEndTime = 0.0f;

	/** 当前技能的伤害是否已应用（用于 DamageAt 延迟触发） */
	bool bDamageApplied = false;
};