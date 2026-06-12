// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraControllerComponent.generated.h"

class UCameraComponent;

/**
 * 摄像机控制器组件
 * 挂载在 WorldPlayerController 上，管理第三人称相机的双轴独立弹性跟随。
 *
 * 核心机制：
 * - 水平（X/Y）和垂直（Z）使用独立的弹性参数
 * - 相机以固定角度和距离位于角色后上方（轴不变）
 * - 玩家移动/上升时，相机从静止加速跟上，停止时惯性滑行
 * - 跳跃期间将 Z 弹性 bypass，相机 Z 轴瞬间跟随角色
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UCameraControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraControllerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ── 相机参数 ──

	/** 相机与角色的水平距离（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "0.0"))
	float ArmLength = 1200.0f;

	/** 相机俯角（度），正值表示从上往下看 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float PitchAngle = 60.0f;

	/** 注视点 Z 偏移（相对于角色脚底） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float LookAtZOffset = 100.0f;

	// ── 水平弹性（X/Y）──

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Horizontal", meta = (ClampMin = "0.1"))
	float HorizontalStiffness = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Horizontal", meta = (ClampMin = "1.0"))
	float HorizontalMaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Horizontal", meta = (ClampMin = "0.1"))
	float HorizontalResponseRate = 6.0f;

	// ── 垂直弹性（Z）──

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Vertical", meta = (ClampMin = "0.1"))
	float VerticalStiffness = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Vertical", meta = (ClampMin = "1.0"))
	float VerticalMaxSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Vertical", meta = (ClampMin = "0.1"))
	float VerticalResponseRate = 8.0f;

	// ── 跳跃相关 ──

	/** 跳跃中是否禁用 Z 轴弹性（直接跟随） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Jump")
	bool bBypassZElasticOnJump = true;

	/** 当前是否处于跳跃状态（由 JumpSkill 设置） */
	UFUNCTION(BlueprintCallable, Category = "Camera|Jump")
	void SetJumping(bool bJumping) { bIsJumping = bJumping; }

	/** 获取当前相机速度（用于调试） */
	UFUNCTION(BlueprintPure, Category = "Camera")
	FVector GetCameraVelocity() const { return CameraVelocity; }

protected:
	virtual void BeginPlay() override;

	/** 当前相机速度（三维） */
	FVector CameraVelocity = FVector::ZeroVector;

	/** 缓存的玩家相机引用 */
	TWeakObjectPtr<UCameraComponent> CachedCamera;

	/** 跳跃状态标记 */
	bool bIsJumping = false;
};