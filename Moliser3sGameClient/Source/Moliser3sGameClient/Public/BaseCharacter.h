// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UAttributeComponent;
class UDamageCalculatorComponent;

/**
 * 基础角色类
 * 仅持有所有角色共有的基础数据（血量、魔法、移动速度等）
 * 子类负责配置具体的移动参数和行为（APlayerCharacter / AEnemyCharacter）
 */
UCLASS(Blueprintable)
class MOLISER3SGAMECLIENT_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	// ===== 移动接口 =====

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

	UFUNCTION(BlueprintPure, Category = "Components")
	UDamageCalculatorComponent* GetDamageCalculator() const { return DamageCalculator; }

	// ===== 速度参数访问 =====

	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetWalkSpeed() const { return WalkSpeed; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetRunSpeed() const { return RunSpeed; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetLockedWalkSpeed() const { return LockedWalkSpeed; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetLockedRunSpeed() const { return LockedRunSpeed; }

	/** 获取解锁距离（注视模式下超过此距离自动切回行走） */
	UFUNCTION(BlueprintPure, Category = "Facing")
	float GetLockOnRange() const { return LockOnRange; }

protected:
	/** 属性组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAttributeComponent> AttributeComponent;

	/** 伤害计算组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDamageCalculatorComponent> DamageCalculator;

	// ===== 非注视模式速度 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom", meta = (ClampMin = "0.0"))
	float WalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom", meta = (ClampMin = "0.0"))
	float RunSpeed = 600.0f;

	// ===== 注视模式速度 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom", meta = (ClampMin = "0.0"))
	float LockedWalkSpeed = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom", meta = (ClampMin = "0.0"))
	float LockedRunSpeed = 300.0f;

	// ===== 注视模式参数 =====
	/** 解锁距离 — 超过此距离自动切回行走模式（0.25米过于敏感，默认10米） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom", meta = (ClampMin = "0.0"))
	float LockOnRange = 1000.0f;

};
