// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UAttributeComponent;

UCLASS(Blueprintable)
class MOLISER3SGAMECLIENT_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	// ===== 移动 =====

	/** 移动角色到指定位置（使用 NavMesh 寻路） */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveToLocation(const FVector& DestLocation);

	/** 停止移动 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopMovement();

	/** 获取当前移动速度（用于动画蓝图） */
	UFUNCTION(BlueprintPure, Category = "Animation")
	float GetSpeed() const { return GetVelocity().Length(); }

	// ===== 组件访问 =====

	UFUNCTION(BlueprintPure, Category = "Components")
	UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

protected:
	/** 移动速度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MoveSpeed = 600.0f;

	/** 属性组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAttributeComponent> AttributeComponent;
};
