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

	// 如果当前有技能正在激活状态，检查 Duration 是否已过期
	if (bSkillActive && CurrentSkill)
	{
		float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		if (Now - CurrentSkillStartTime >= CurrentSkill->Duration)
		{
			// 技能持续时间已结束，清空当前技能状态
			// 注意：不清除 QueueIndex，保留当前位置，等下次 ActivateNextSkill 时才会移动到下一个
			CurrentSkill = nullptr;
			bSkillActive = false;
			CurrentSkillStartTime = 0.0f;
		}
	}
}

void USkillSystemComponent::ActivateNextSkill()
{
	// 如果 SkillQueue 为空但 SkillList 有内容，自动用 SkillList 填充队列
	if (SkillQueue.Num() == 0)
	{
		if (SkillList.Num() > 0)
		{
			SkillQueue = SkillList;
			QueueIndex = 0;
			UE_LOG(LogTemp, Log, TEXT("SkillSystemComponent::ActivateNextSkill — auto-populated SkillQueue from SkillList (%d skills)"), SkillList.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillSystemComponent::ActivateNextSkill — SkillQueue and SkillList are both empty!"));
			return;
		}
	}

	// 如果当前技能还在激活状态，不能释放下一个
	if (bSkillActive)
	{
		UE_LOG(LogTemp, Verbose, TEXT("SkillSystemComponent::ActivateNextSkill — skill still active, ignored."));
		return;
	}

	// 确保索引有效
	if (!SkillQueue.IsValidIndex(QueueIndex))
	{
		QueueIndex = 0;
	}

	USkillBase* Skill = SkillQueue[QueueIndex];
	if (!Skill)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSystemComponent::ActivateNextSkill — skill at index %d is null, skipping."), QueueIndex);
		// 当前索引技能无效，尝试移动到下一个
		QueueIndex = (QueueIndex + 1) % SkillQueue.Num();
		return;
	}

	// 执行技能
	CurrentSkill = Skill;
	CurrentSkillStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	bSkillActive = true;

	UE_LOG(LogTemp, Log, TEXT("SkillSystemComponent::ActivateNextSkill — executing skill '%s' (QueueIndex=%d)"),
		*Skill->SkillName.ToString(), QueueIndex);

	Skill->Execute(GetOwner());

	// 移动到队列中的下一个位置，下次调用 ActivateNextSkill 时释放下一个
	QueueIndex = (QueueIndex + 1) % SkillQueue.Num();
}

void USkillSystemComponent::AddSkill(USkillBase* NewSkill)
{
	if (NewSkill)
	{
		SkillList.Add(NewSkill);
	}
}

void USkillSystemComponent::SetSkillQueue(const TArray<USkillBase*>& InQueue)
{
	SkillQueue = InQueue;
	QueueIndex = 0;
	CurrentSkill = nullptr;
	bSkillActive = false;
	CurrentSkillStartTime = 0.0f;
}