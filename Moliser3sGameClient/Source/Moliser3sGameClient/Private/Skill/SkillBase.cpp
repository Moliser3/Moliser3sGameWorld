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

	// 如果指定了槽位名称，停止该槽位上正在播放的蒙太奇
	if (!MontageSlotName.IsNone())
	{
		AnimInst->Montage_Stop(0.2f, SkillMontage);
	}

	// 播放技能蒙太奇
	AnimInst->Montage_Play(SkillMontage, 1.0f);
}