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

	if (bSkillActive && CurrentSkill)
	{
		float Elapsed = Now - CurrentSkillStartTime;

		if (!bDamageApplied && Elapsed >= CurrentSkill->DamageAt)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] DamageAt triggered for '%s' (elapsed=%.2f, DamageAt=%.2f)"),
				*CurrentSkill->SkillName.ToString(), Elapsed, CurrentSkill->DamageAt);

			CurrentSkill->ApplyDamage(GetOwner());
			bDamageApplied = true;
		}

		if (Elapsed >= CurrentSkill->Duration)
		{
			CurrentSkill = nullptr;
			bSkillActive = false;
			CurrentSkillStartTime = 0.0f;
			bDamageApplied = false;

			bInComboWindow = true;
			ComboWindowEndTime = Now + ComboWindowDuration;
		}
		else
		{
			if (bSkillActive && CurrentSkill)
			{
				CurrentSkill->Update(GetOwner(), DeltaTime);
			}
		}
	}

	if (bInComboWindow)
	{
		if (Now >= ComboWindowEndTime)
		{
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
			SkillQueue.Empty();
			for (USkillBase* S : SkillList)
			{
				// 过滤空指针 + 过滤 SkillName 为空的无效技能实例（蓝图序列化残留）
				if (S && !S->SkillName.IsNone())
				{
					SkillQueue.Add(S);
				}
			}
			QueueIndex = 0;

			if (SkillQueue.Num() == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill FAILED — no valid skills after filtering!"));
				return;
			}
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
		float Elapsed = Now - CurrentSkillStartTime;

		if (Elapsed < CurrentSkill->GetInterruptibleAt())
		{
			// 不可打断：忽略本次点击
			return;
		}

		// 可打断
		CurrentSkill->OnInterrupt(GetOwner());

		CurrentSkill = nullptr;
		bSkillActive = false;
		CurrentSkillStartTime = 0.0f;
		bDamageApplied = false;
		bInComboWindow = false;
		ComboWindowEndTime = 0.0f;

		if (!SkillQueue.IsValidIndex(QueueIndex))
		{
			QueueIndex = 0;
		}

		goto ExecuteSkill;
	}
	else if (bSkillActive && !CurrentSkill)
	{
		bSkillActive = false;
	}
	else if (!bInComboWindow)
	{
		// IDLE 状态：保留 QueueIndex
	}
	else
	{
		// COMBO_WINDOW 状态：继续下一个技能
	}

ExecuteSkill:
	if (!SkillQueue.IsValidIndex(QueueIndex))
	{
		QueueIndex = 0;
	}

	// 循环查找有效技能（跳过空指针 + 跳过 SkillName 为空的无名技能）
	USkillBase* Skill = nullptr;
	int32 CheckedCount = 0;
	while (CheckedCount < SkillQueue.Num())
	{
		Skill = SkillQueue[QueueIndex];
		if (Skill && !Skill->SkillName.IsNone())
		{
			break;
		}
		QueueIndex = (QueueIndex + 1) % SkillQueue.Num();
		CheckedCount++;
	}

	if (!Skill || Skill->SkillName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] FAILED — no valid skill found in queue!"));
		return;
	}

	CurrentSkill = Skill;
	CurrentSkillStartTime = Now;
	bSkillActive = true;
	bDamageApplied = false;
	bInComboWindow = false;
	ComboWindowEndTime = 0.0f;

	Skill->Execute(GetOwner());

	QueueIndex = (QueueIndex + 1) % SkillQueue.Num();
}

void USkillSystemComponent::AddSkill(USkillBase* NewSkill)
{
	if (NewSkill)
	{
		SkillList.Add(NewSkill);
	}
}

float USkillSystemComponent::GetCurrentSkillElapsed() const
{
	if (!bSkillActive || !CurrentSkill)
	{
		return -1.0f;
	}
	float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return Now - CurrentSkillStartTime;
}

float USkillSystemComponent::GetMaxAttackRange() const
{
	const TArray<TObjectPtr<USkillBase>>* ActiveQueue = &SkillQueue;
	if (ActiveQueue->Num() == 0)
	{
		ActiveQueue = &SkillList;
	}
	if (ActiveQueue->Num() == 0)
	{
		return -1.0f;
	}

	for (int32 i = 0; i < ActiveQueue->Num(); i++)
	{
		USkillBase* Skill = (*ActiveQueue)[i];
		if (Skill && Skill->MaxAttackRange > 0)
		{
			return Skill->MaxAttackRange;
		}
	}

	return -1.0f;
}

USkillBase* USkillSystemComponent::PeekNextSkill() const
{
	const TArray<TObjectPtr<USkillBase>>* ActiveQueue = &SkillQueue;
	if (ActiveQueue->Num() == 0)
	{
		ActiveQueue = &SkillList;
	}
	if (ActiveQueue->Num() == 0)
	{
		return nullptr;
	}

	int32 Index = QueueIndex;
	if (!ActiveQueue->IsValidIndex(Index))
	{
		Index = 0;
	}

	int32 Checked = 0;
	USkillBase* Skill = nullptr;
	while (Checked < ActiveQueue->Num())
	{
		Skill = (*ActiveQueue)[Index];
		if (Skill)
		{
			break;
		}
		Index = (Index + 1) % ActiveQueue->Num();
		Checked++;
	}

	return Skill;
}

void USkillSystemComponent::TryInterruptCurrentSkill()
{
	if (!bSkillActive || !CurrentSkill)
	{
		return;
	}

	float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	float Elapsed = Now - CurrentSkillStartTime;

	if (Elapsed < CurrentSkill->GetInterruptibleAt())
	{
		return;
	}

	CurrentSkill->OnInterrupt(GetOwner());

	CurrentSkill = nullptr;
	bSkillActive = false;
	CurrentSkillStartTime = 0.0f;
	bDamageApplied = false;
	bInComboWindow = false;
	ComboWindowEndTime = 0.0f;
}

void USkillSystemComponent::ForceEndCurrentSkill()
{
	if (!bSkillActive)
	{
		return;
	}

	CurrentSkill = nullptr;
	bSkillActive = false;
	CurrentSkillStartTime = 0.0f;
	bDamageApplied = false;

	bInComboWindow = false;
	ComboWindowEndTime = 0.0f;
	QueueIndex = 0;
}

bool USkillSystemComponent::IsNextSkillMovement() const
{
	USkillBase* Skill = PeekNextSkill();
	return Skill ? Skill->bIsMovementSkill : false;
}

void USkillSystemComponent::SetSkillQueue(const TArray<USkillBase*>& InQueue)
{
	SkillQueue.Empty();
	for (USkillBase* S : InQueue)
	{
		if (S)
		{
			SkillQueue.Add(S);
		}
	}
	QueueIndex = 0;
	CurrentSkill = nullptr;
	bSkillActive = false;
	bInComboWindow = false;
	ComboWindowEndTime = 0.0f;
	CurrentSkillStartTime = 0.0f;
	bDamageApplied = false;
}

// force recompile marker