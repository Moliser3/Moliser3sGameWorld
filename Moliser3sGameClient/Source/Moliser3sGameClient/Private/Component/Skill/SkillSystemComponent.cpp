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

	if (SkillPhase == ESkillPhase::Windup && CurrentSkill)
	{
		float Elapsed = Now - PhaseStartTime;

		CurrentSkill->OnWindupUpdate(GetOwner(), DeltaTime);

		if (Elapsed >= CurrentSkill->GetWindupTime())
		{
			CurrentSkill->OnExecute(GetOwner());
			SkillPhase = ESkillPhase::Recovery;
			PhaseStartTime = Now;
		}
	}
	else if (SkillPhase == ESkillPhase::Recovery && CurrentSkill)
	{
		float Elapsed = Now - PhaseStartTime;

		CurrentSkill->OnRecoveryUpdate(GetOwner(), DeltaTime);

		if (Elapsed >= CurrentSkill->GetRecoveryTime())
		{
			CachedLinkDuration = CurrentSkill->GetCustomLinkTime();

			CurrentSkill = nullptr;
			SkillPhase = ESkillPhase::LinkWindow;
			PhaseStartTime = Now;
		}
	}
	else if (SkillPhase == ESkillPhase::LinkWindow)
	{
		float Elapsed = Now - PhaseStartTime;

		if (Elapsed >= CachedLinkDuration)
		{
			DEBUG_SKILL("[技能] 衔接超时, 重置索引");
			LeftGroupIndex = 0;
			RightGroupIndex = 0;
			LeftLastSkillType = ESkillType::None;
			LeftStageForType = 0;
			RightLastSkillType = ESkillType::None;
			RightStageForType = 0;
			SkillPhase = ESkillPhase::Idle;
		}
	}
}

void USkillSystemComponent::ActivateLeft()
{
	ExecuteSkillFromGroup(LeftSkillGroup, LeftGroupIndex, LeftLastSkillType, LeftStageForType);
}

void USkillSystemComponent::ActivateRight()
{
	ExecuteSkillFromGroup(RightSkillGroup, RightGroupIndex, RightLastSkillType, RightStageForType);
}

void USkillSystemComponent::ExecuteSkillFromGroup(
	TArray<TObjectPtr<USkillBase>>& Group, int32& Index,
	ESkillType& LastType, int32& StageForType)
{
	float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (Group.Num() == 0)
	{
		return;
	}

	switch (SkillPhase)
	{
	case ESkillPhase::Windup:
		return;

	case ESkillPhase::Recovery:
		if (CurrentSkill)
		{
			DEBUG_SKILL("[技能] 打断: %s 阶段%d", *CurrentSkill->SkillName.ToString(), CurrentSkill->GetCurrentStage());
			CurrentSkill->OnInterrupt(GetOwner());
			CurrentSkill = nullptr;
		}
		SkillPhase = ESkillPhase::Idle;
		break;

	case ESkillPhase::Idle:
	case ESkillPhase::LinkWindow:
		break;
	}

	if (!Group.IsValidIndex(Index))
	{
		Index = 0;
	}

	USkillBase* Skill = Group[Index];
	if (!Skill)
	{
		Index = 0;
		return;
	}

	if (Skill->SkillType == LastType)
	{
		StageForType++;
	}
	else
	{
		StageForType = 0;
	}
	LastType = Skill->SkillType;

	Skill->SetCurrentStage(StageForType);
	CurrentSkill = Skill;
	SkillPhase = ESkillPhase::Windup;
	PhaseStartTime = Now;

	DEBUG_SKILL("[技能] 释放: %s 阶段%d (组%d)", *Skill->SkillName.ToString(), StageForType, Index);

	Skill->Execute(GetOwner());

	Index = (Index + 1) % Group.Num();
	DEBUG_SKILL("[技能] 组索引→%d", Index);
}

float USkillSystemComponent::GetMaxSkillRange() const
{
	auto FindMaxRange = [](const TArray<TObjectPtr<USkillBase>>& Group) -> float
	{
		for (const auto& Skill : Group)
		{
			if (Skill && Skill->MaxSkillRange > 0)
			{
				return Skill->MaxSkillRange;
			}
		}
		return -1.0f;
	};

	float LeftRange = FindMaxRange(LeftSkillGroup);
	float RightRange = FindMaxRange(RightSkillGroup);
	return FMath::Max(LeftRange, RightRange);
}
