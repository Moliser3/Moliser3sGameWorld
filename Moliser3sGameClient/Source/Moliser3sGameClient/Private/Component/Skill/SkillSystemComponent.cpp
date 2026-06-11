// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Skill/SkillSystemComponent.h"
#include "Skill/SkillBase.h"
#include "Engine/World.h"

USkillSystemComponent::USkillSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USkillSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// ── 状态一：ACTIVE — 检测 DamageAt / Duration ──
	if (bSkillActive && CurrentSkill)
	{
		float Elapsed = Now - CurrentSkillStartTime;

		// 检测 DamageAt 时间点，触发延迟伤害
		if (!bDamageApplied && Elapsed >= CurrentSkill->DamageAt)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] DamageAt triggered for '%s' (elapsed=%.2f, DamageAt=%.2f)"),
				*CurrentSkill->SkillName.ToString(), Elapsed, CurrentSkill->DamageAt);

			CurrentSkill->ApplyDamage(GetOwner());
			bDamageApplied = true;
		}

		// 检测 Duration 是否到期
		if (Elapsed >= CurrentSkill->Duration)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ACTIVE → COMBO_WINDOW  skill='%s'  elapsed=%.2f  duration=%.2f"),
				*CurrentSkill->SkillName.ToString(), Elapsed, CurrentSkill->Duration);

			CurrentSkill = nullptr;
			bSkillActive = false;
			CurrentSkillStartTime = 0.0f;
			bDamageApplied = false;

			bInComboWindow = true;
			ComboWindowEndTime = Now + ComboWindowDuration;

			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] COMBO_WINDOW started  endTime=%.2f  window=%.2fs"),
				ComboWindowEndTime, ComboWindowDuration);
		}
	}

	// ── 状态二：COMBO_WINDOW — 检测窗口是否到期 ──
	if (bInComboWindow)
	{
		if (Now >= ComboWindowEndTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] COMBO_WINDOW expired → reset QueueIndex to 0"));

			bInComboWindow = false;
			ComboWindowEndTime = 0.0f;
			QueueIndex = 0;
		}
	}
}

void USkillSystemComponent::ActivateNextSkill()
{
	float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// ── 第一步：确保队列有内容 ──
	if (SkillQueue.Num() == 0)
	{
		if (SkillList.Num() > 0)
		{
			SkillQueue = SkillList;
			QueueIndex = 0;
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] auto-populated SkillQueue from SkillList (%d skills)"), SkillList.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill FAILED — SkillQueue and SkillList are both empty!"));
			return;
		}
	}

	// ── 第二步：状态检查 ──

	if (bSkillActive && CurrentSkill)
	{
		// ACTIVE 状态：检查是否可打断
		float Elapsed = Now - CurrentSkillStartTime;
		if (Elapsed < CurrentSkill->InterruptibleAt)
		{
			// 不可打断：忽略本次点击
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill IGNORED — ACTIVE (elapsed=%.2f < InterruptibleAt=%.2f)"),
				Elapsed, CurrentSkill->InterruptibleAt);
			return;
		}

		// 可打断：提前结束当前技能，然后执行下一个
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill INTERRUPT — breaking current skill '%s' (elapsed=%.2f >= InterruptibleAt=%.2f)"),
			*CurrentSkill->SkillName.ToString(), Elapsed, CurrentSkill->InterruptibleAt);

		// 清理当前技能状态
		CurrentSkill = nullptr;
		bSkillActive = false;
		CurrentSkillStartTime = 0.0f;
		bDamageApplied = false;
		bInComboWindow = false;
		ComboWindowEndTime = 0.0f;
	}
	else if (bSkillActive && !CurrentSkill)
	{
		// 异常状态：技能激活但没有 CurrentSkill
		bSkillActive = false;
	}
	else if (!bInComboWindow)
	{
		// IDLE 状态：从头开始
		QueueIndex = 0;
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill from IDLE — reset QueueIndex to 0"));
	}
	else
	{
		// COMBO_WINDOW 状态：继续下一个技能
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill from COMBO_WINDOW — continuing combo at QueueIndex=%d"), QueueIndex);
	}

	// ── 第三步：读取技能 ──
	if (!SkillQueue.IsValidIndex(QueueIndex))
	{
		QueueIndex = 0;
	}

	USkillBase* Skill = SkillQueue[QueueIndex];
	if (!Skill)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill FAILED — skill at QueueIndex=%d is null, skipping to next"), QueueIndex);
		QueueIndex = (QueueIndex + 1) % SkillQueue.Num();
		return;
	}

	// ── 第四步：执行技能 ──
	CurrentSkill = Skill;
	CurrentSkillStartTime = Now;
	bSkillActive = true;
	bDamageApplied = false;

	// 退出连招窗口（因为进入了 ACTIVE）
	bInComboWindow = false;
	ComboWindowEndTime = 0.0f;

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] EXECUTING skill='%s'  QueueIndex=%d  Duration=%.2fs  DamageAt=%.2f  InterruptibleAt=%.2f"),
		*Skill->SkillName.ToString(), QueueIndex, Skill->Duration, Skill->DamageAt, Skill->InterruptibleAt);

	Skill->Execute(GetOwner());

	// ── 第五步：移动到队列下一个位置 ──
	QueueIndex = (QueueIndex + 1) % SkillQueue.Num();

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] advanced to QueueIndex=%d (next skill)"), QueueIndex);
}

void USkillSystemComponent::AddSkill(USkillBase* NewSkill)
{
	if (NewSkill)
	{
		SkillList.Add(NewSkill);
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] added skill '%s' to SkillList (total=%d)"),
			*NewSkill->SkillName.ToString(), SkillList.Num());
	}
}

void USkillSystemComponent::SetSkillQueue(const TArray<USkillBase*>& InQueue)
{
	SkillQueue = InQueue;
	QueueIndex = 0;
	CurrentSkill = nullptr;
	bSkillActive = false;
	bInComboWindow = false;
	ComboWindowEndTime = 0.0f;
	CurrentSkillStartTime = 0.0f;
	bDamageApplied = false;

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] SetSkillQueue called — %d skills, QueueIndex=0"), InQueue.Num());
}