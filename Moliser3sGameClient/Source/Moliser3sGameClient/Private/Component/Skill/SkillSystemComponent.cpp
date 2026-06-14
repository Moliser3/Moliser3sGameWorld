// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Skill/SkillSystemComponent.h"
#include "Skill/SkillBase.h"
#include "Skill/DamageSkillBase.h"
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

	// ── 前摇阶段 ──
	if (SkillPhase == ESkillPhase::Windup && CurrentSkill)
	{
		float Elapsed = Now - PhaseStartTime;

		CurrentSkill->OnWindupUpdate(GetOwner(), DeltaTime);

		if (Elapsed >= CurrentSkill->WindupTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] OnExecute for '%s'"),
				*CurrentSkill->SkillName.ToString());

			// 前摇结束 → 技能触发
			CurrentSkill->OnExecute(GetOwner());
			SkillPhase = ESkillPhase::Recovery;
			PhaseStartTime = Now;
		}
	}
	// ── 后摇阶段 ──
	else if (SkillPhase == ESkillPhase::Recovery && CurrentSkill)
	{
		float Elapsed = Now - PhaseStartTime;

		CurrentSkill->OnRecoveryUpdate(GetOwner(), DeltaTime);

		if (Elapsed >= CurrentSkill->RecoveryTime)
		{
			// 缓存在 CurrentSkill 置空前保存衔接时间
			CachedLinkDuration = CurrentSkill->CustomLinkTime;

			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] Recovery done for '%s', link window (%.2f s)"),
				*CurrentSkill->SkillName.ToString(), CachedLinkDuration);

			CurrentSkill = nullptr;
			SkillPhase = ESkillPhase::LinkWindow;
			PhaseStartTime = Now;
		}
	}
	// ── 衔接阶段（后摇结束后的额外等待期）──
	else if (SkillPhase == ESkillPhase::LinkWindow)
	{
		float Elapsed = Now - PhaseStartTime;

		if (Elapsed >= CachedLinkDuration)
		{
			// 衔接超时，重置技能组索引到第一个
			GroupSkillIndex = 0;
			SkillPhase = ESkillPhase::Idle;

			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] Link window expired, reset index to 0"));
		}
	}
}

void USkillSystemComponent::ActivateNextSkill()
{
	float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (SkillGroup.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill FAILED — SkillGroup is empty!"));
		return;
	}

	// ── 状态检查 ──
	switch (SkillPhase)
	{
	case ESkillPhase::Windup:
		// 前摇不可打断
		return;

	case ESkillPhase::Recovery:
		// 后摇可打断 → 打断当前技能
		if (CurrentSkill)
		{
			CurrentSkill->OnInterrupt(GetOwner());
			CurrentSkill = nullptr;
		}
		SkillPhase = ESkillPhase::Idle;
		break;

	case ESkillPhase::Idle:
	case ESkillPhase::LinkWindow:
		break;
	}

	// ── 组内取下一个有效技能 ──
	int32 Index = GroupSkillIndex;
	if (!SkillGroup.IsValidIndex(Index))
	{
		Index = 0;
		GroupSkillIndex = 0;
	}

	USkillBase* Skill = SkillGroup[Index];
	if (!Skill || Skill->SkillName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] FAILED — no valid skill at index %d!"), Index);
		return;
	}

	// 执行技能
	CurrentSkill = Skill;
	SkillPhase = ESkillPhase::Windup;
	PhaseStartTime = Now;

	Skill->Execute(GetOwner());

	// 推进组内索引（技能组只有 1 个时，索引永远指向自己）
	if (SkillGroup.Num() > 1)
	{
		GroupSkillIndex = (GroupSkillIndex + 1) % SkillGroup.Num();
	}
	else
	{
		GroupSkillIndex = 0;
	}
}

void USkillSystemComponent::AddSkill(USkillBase* NewSkill)
{
	if (NewSkill)
	{
		SkillGroup.Add(NewSkill);
	}
}

float USkillSystemComponent::GetMaxSkillRange() const
{
	if (SkillGroup.Num() == 0)
	{
		return -1.0f;
	}

	for (int32 i = 0; i < SkillGroup.Num(); i++)
	{
		USkillBase* Skill = SkillGroup[i];
		if (Skill && Skill->MaxSkillRange > 0)
		{
			return Skill->MaxSkillRange;
		}
	}

	return -1.0f;
}

USkillBase* USkillSystemComponent::PeekNextSkill() const
{
	if (SkillGroup.Num() == 0)
	{
		return nullptr;
	}

	int32 Index = GroupSkillIndex;
	if (!SkillGroup.IsValidIndex(Index))
	{
		return nullptr;
	}

	return SkillGroup[Index];
}

ESkillCategory USkillSystemComponent::GetNextSkillCategory() const
{
	USkillBase* Skill = PeekNextSkill();
	return Skill ? Skill->SkillCategory : ESkillCategory::Attack;
}
