// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/SkillBase.h"
#include "JumpSkill.generated.h"

/**
 * 跳跃技能
 * 使角色向上跳跃，可融入技能循环队列中
 * 不造成伤害（ApplyDamage 空实现）
 * 通过 InterruptibleAt 控制是否可在空中打断（默认设为大值=不可打断）
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UJumpSkill : public USkillBase
{
	GENERATED_BODY()

public:
	virtual void Execute(AActor* Instigator) override;
	virtual void ApplyDamage(AActor* Instigator) override {}
};