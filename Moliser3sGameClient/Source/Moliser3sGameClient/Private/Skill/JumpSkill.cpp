// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/JumpSkill.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

void UJumpSkill::Execute(AActor* Instigator)
{
	if (!Instigator)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar)
	{
		return;
	}

	// 如果角色在空中，不重复跳跃
	if (OwnerChar->GetCharacterMovement() && OwnerChar->GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// 技能释放时停止移动
	if (AController* Ctl = OwnerChar->GetController())
	{
		Ctl->StopMovement();
	}

	// 播放跳跃蒙太奇（如果已配置）
	PlaySkillMontage(Instigator);

	// 跳跃
	OwnerChar->Jump();
}