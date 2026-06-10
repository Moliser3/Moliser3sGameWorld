// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "EnemyCharacter.generated.h"

/**
 * 敌人类
 * 继承自 ABaseCharacter，配置适合 AI 控制的移动参数
 * 由 AAIController 驱动
 * 后续扩展：AI 行为、感知系统、攻击逻辑等
 */
UCLASS(Blueprintable)
class MOLISER3SGAMECLIENT_API AEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;
};
