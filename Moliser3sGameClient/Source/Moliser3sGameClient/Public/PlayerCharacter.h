// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "PlayerCharacter.generated.h"

class UFacingComponent;
class USkillSystemComponent;

/**
 * 玩家角色类
 * 继承自 ABaseCharacter，配置玩家的移动参数
 * 由 APlayerController（AWorldPlayerController）控制
 * 持有 UFacingComponent 管理行走/注视朝向
 * 注视模式下速度自动切换（速度参数在基类 BaseCharacter 中配置）
 */
UCLASS(Blueprintable)
class MOLISER3SGAMECLIENT_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	/** 获取朝向控制组件 */
	UFUNCTION(BlueprintPure, Category = "Components")
	UFacingComponent* GetFacingComponent() const { return FacingComponent; }

	/** 获取技能系统组件 */
	UFUNCTION(BlueprintPure, Category = "Components")
	USkillSystemComponent* GetSkillSystem() const { return SkillSystemComponent; }

protected:
	virtual void BeginPlay() override;

	/** 更新移动速度（根据当前朝向模式） */
	UFUNCTION()
	void UpdateMovementSpeed(EFacingMode NewMode);

	/** 朝向控制组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFacingComponent> FacingComponent;

	/** 技能系统组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkillSystemComponent> SkillSystemComponent;
};
