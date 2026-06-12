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
		else
		{
			// 每帧更新当前技能（如跳跃抛物线位移、持续性技能等）
			// 放在 Duration 检测之后，避免 Update 中 EndJump 导致 CurrentSkill 置空后
			// 继续访问 CurrentSkill 造成崩溃
			if (bSkillActive && CurrentSkill)
			{
				CurrentSkill->Update(GetOwner(), DeltaTime);
			}
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

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill called — bSkillActive=%d bInComboWindow=%d QueueIndex=%d"),
		bSkillActive, bInComboWindow, QueueIndex);

	// ── 第一步：确保队列有内容 ──
	if (SkillQueue.Num() == 0)
	{
		if (SkillList.Num() > 0)
		{
			// 遍历 SkillList，只加入有效技能（过滤掉蓝图数组中的空指针残留）
			SkillQueue.Empty();
			for (USkillBase* S : SkillList)
			{
				if (S)
				{
					SkillQueue.Add(S);
				}
			}
			QueueIndex = 0;
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] auto-populated SkillQueue from SkillList: %d valid skills (filtered from %d total)"),
				SkillQueue.Num(), SkillList.Num());
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
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ACTIVE check — elapsed=%.2f InterruptibleAt=%.2f"),
			Elapsed, CurrentSkill->GetInterruptibleAt());

		if (Elapsed < CurrentSkill->GetInterruptibleAt())
		{
			// 不可打断：忽略本次点击
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill IGNORED — skill still in uninterruptible phase"));
			return;
		}

		// 可打断：提前结束当前技能，然后执行下一个
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill INTERRUPT — cancelling current skill, executing next"));

		// 通知当前技能：被打断了（让技能清理运行时状态，如跳跃的 bIsJumping）
		CurrentSkill->OnInterrupt(GetOwner());

		// 清理当前技能状态
		CurrentSkill = nullptr;
		bSkillActive = false;
		CurrentSkillStartTime = 0.0f;
		bDamageApplied = false;
		bInComboWindow = false;
		ComboWindowEndTime = 0.0f;

		// QueueIndex 已经在之前 ++ 过了，指向下一个技能的位置
		// 如果队列已循环一圈（到了最后一个技能后面），就回到 0
		if (!SkillQueue.IsValidIndex(QueueIndex))
		{
			QueueIndex = 0;
		}

		// 打断后直接跳到执行阶段，不经过下面的状态检查（防止 IDLE 分支重置 QueueIndex=0）
		goto ExecuteSkill;
	}
	else if (bSkillActive && !CurrentSkill)
	{
		// 异常状态：技能激活但没有 CurrentSkill
		bSkillActive = false;
	}
	else if (!bInComboWindow)
	{
		// IDLE 状态：保留当前 QueueIndex，使用队列中的下一个技能
		// 注意：不重置 QueueIndex=0，否则打断跳跃收尾后无法执行跳跃后的下一个技能
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill from IDLE — QueueIndex=%d"), QueueIndex);
	}
	else
	{
		// COMBO_WINDOW 状态：继续下一个技能
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill from COMBO_WINDOW — continuing combo at QueueIndex=%d"), QueueIndex);
	}

ExecuteSkill:
	// ── 第三步：读取技能 ──
	if (!SkillQueue.IsValidIndex(QueueIndex))
	{
		QueueIndex = 0;
	}

	// ── 循环查找有效技能（跳过队列中的空指针）──
	USkillBase* Skill = nullptr;
	int32 CheckedCount = 0;
	while (CheckedCount < SkillQueue.Num())
	{
		Skill = SkillQueue[QueueIndex];
		if (Skill)
		{
			break;
		}
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill — skill at QueueIndex=%d is null, skipping"), QueueIndex);
		QueueIndex = (QueueIndex + 1) % SkillQueue.Num();
		CheckedCount++;
	}

	if (!Skill)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ActivateNextSkill FAILED — no valid skill found in queue!"));
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

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ==== EXECUTING skill='%s' QueueIndex=%d ===="),
		*Skill->SkillName.ToString(), QueueIndex);

	Skill->Execute(GetOwner());

	// ── 第五步：移动到队列下一个位置 ──
	int32 OldIndex = QueueIndex;
	QueueIndex = (QueueIndex + 1) % SkillQueue.Num();

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] advanced from QueueIndex=%d to %d (next skill)"), OldIndex, QueueIndex);
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
	// 获取当前队列指针
	const TArray<TObjectPtr<USkillBase>>* ActiveQueue = &SkillQueue;
	if (ActiveQueue->Num() == 0)
	{
		ActiveQueue = &SkillList;
	}
	if (ActiveQueue->Num() == 0)
	{
		return -1.0f;
	}

	// 遍历队列查找第一个有有效 MaxAttackRange 的技能
	for (int32 i = 0; i < ActiveQueue->Num(); i++)
	{
		USkillBase* Skill = (*ActiveQueue)[i];
		if (Skill && Skill->MaxAttackRange > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] GetMaxAttackRange — skill='%s' MaxAttackRange=%.0f"),
				*Skill->SkillName.ToString(), Skill->MaxAttackRange);
			return Skill->MaxAttackRange;
		}
	}

	// 没有近战技能 → 全远程
	return -1.0f;
}

USkillBase* USkillSystemComponent::PeekNextSkill() const
{
	// 获取活跃队列（优先 SkillQueue，其次 SkillList）
	const TArray<TObjectPtr<USkillBase>>* ActiveQueue = &SkillQueue;
	if (ActiveQueue->Num() == 0)
	{
		ActiveQueue = &SkillList;
	}
	if (ActiveQueue->Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] PeekNextSkill — SkillQueue.Num=%d SkillList.Num=%d → nullptr"), 
			SkillQueue.Num(), SkillList.Num());
		return nullptr;
	}

	// QueueIndex 始终指向下一次要执行的技能：
	// - IDLE 状态：QueueIndex=0，指向第一个技能
	// - ACTIVE 状态：QueueIndex 已自增，指向当前技能的下一个
	// - COMBO_WINDOW 状态：QueueIndex 已自增，指向连招的下一个
	int32 Index = QueueIndex;
	if (!ActiveQueue->IsValidIndex(Index))
	{
		Index = 0;
	}

	// 从当前索引开始，循环查找第一个有效技能（跳过空指针）
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

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] PeekNextSkill — QueIdx=%d QueueSize=%d ListSize=%d → Skill=%s"),
		QueueIndex, SkillQueue.Num(), SkillList.Num(), Skill ? *Skill->SkillName.ToString() : TEXT("null"));
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

	// 检查是否可打断
	if (Elapsed < CurrentSkill->GetInterruptibleAt())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] TryInterrupt — IGNORED (elapsed=%.2f < InterruptibleAt=%.2f)"),
			Elapsed, CurrentSkill->GetInterruptibleAt());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] TryInterrupt — interrupting skill='%s'"), *CurrentSkill->SkillName.ToString());

	// 通知当前技能被打断
	CurrentSkill->OnInterrupt(GetOwner());

	// 清理技能状态，回到 IDLE
	// 注意：不重置 QueueIndex，保留队列当前位置指向下一个技能
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

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ForceEndCurrentSkill — skill='%s'"),
		CurrentSkill ? *CurrentSkill->SkillName.ToString() : TEXT("nullptr"));

	CurrentSkill = nullptr;
	bSkillActive = false;
	CurrentSkillStartTime = 0.0f;
	bDamageApplied = false;

	// 不进入连招窗口，直接回到 IDLE（适用于跳跃等自行管理时长、不需连招的技能）
	bInComboWindow = false;
	ComboWindowEndTime = 0.0f;
	QueueIndex = 0;

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] ForceEnd → IDLE (QueueIndex reset to 0)"));
}

bool USkillSystemComponent::IsNextSkillMovement() const
{
	USkillBase* Skill = PeekNextSkill();
	return Skill ? Skill->bIsMovementSkill : false;
}

void USkillSystemComponent::SetSkillQueue(const TArray<USkillBase*>& InQueue)
{
	// 过滤空指针后填充队列
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

	UE_LOG(LogTemp, Warning, TEXT("[SkillSystem] SetSkillQueue called — %d in, %d valid skills, QueueIndex=0"),
		InQueue.Num(), SkillQueue.Num());
}

// force recompile marker