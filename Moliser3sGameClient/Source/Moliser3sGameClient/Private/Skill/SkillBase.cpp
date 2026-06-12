// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/SkillBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

void USkillBase::Execute(AActor* Instigator)
{
	// 基类空实现，子类重写
}

void USkillBase::Update(AActor* Instigator, float DeltaTime)
{
	// 基类空实现，子类重写
}

void USkillBase::OnInterrupt(AActor* Instigator)
{
	// 基类空实现，子类重写
}

void USkillBase::ApplyDamage(AActor* Instigator)
{
	// 基类空实现，子类重写
}

void USkillBase::PlaySkillMontage(AActor* Instigator)
{
	if (!Instigator || !SkillMontage)
	{
		return;
	}

	ACharacter* Char = Cast<ACharacter>(Instigator);
	if (!Char)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Char->GetMesh();
	if (!Mesh)
	{
		return;
	}

	UAnimInstance* AnimInst = Mesh->GetAnimInstance();
	if (!AnimInst)
	{
		return;
	}

	// 停止当前槽位上正在播放的蒙太奇，确保新技能动画能正常播放
	// 不依赖 MontageSlotName 判断，因为跳跃技能等可能没有设置槽位名
	// 但前一个技能（如出拳）的蒙太奇可能仍在槽位上残留，阻止新蒙太奇播放
	AnimInst->Montage_Stop(0.2f, SkillMontage);

	// 播放技能蒙太奇
	AnimInst->Montage_Play(SkillMontage, 1.0f);
}