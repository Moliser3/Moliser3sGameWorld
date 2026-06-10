// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FacingComponent.generated.h"

/**
 * 朝向模式
 */
UENUM(BlueprintType)
enum class EFacingMode : uint8
{
	Walking UMETA(DisplayName = "行走模式"),	// 面朝移动方向（由 CharacterMovement 控制）
	Aiming  UMETA(DisplayName = "注视模式")		// 面朝锁定目标
};

/**
 * 4方向枚举
 * 相对于角色正面的方向
 * Forward, Right, Back, Left
 */
UENUM(BlueprintType)
enum class E4Direction : uint8
{
	Forward UMETA(DisplayName = "Forward"),
	Right   UMETA(DisplayName = "Right"),
	Back    UMETA(DisplayName = "Back"),
	Left    UMETA(DisplayName = "Left")
};

/** 朝向模式改变时广播的事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFacingModeChangedDelegate, EFacingMode, NewMode);

/**
 * 朝向控制组件
 * 挂载到角色上，管理行走/注视两种朝向模式的切换
 * 注视模式下自动朝向目标，超过最大距离自动切回行走
 * 提供4方向移动判定供动画蓝图使用
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UFacingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFacingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== 公共接口 =====

	/** 设置注视目标，进入注视模式 */
	UFUNCTION(BlueprintCallable, Category = "Facing")
	void SetAimTarget(AActor* Target);

	/** 清除注视目标，回到行走模式 */
	UFUNCTION(BlueprintCallable, Category = "Facing")
	void ClearAimTarget();

	/** 获取当前朝向模式 */
	UFUNCTION(BlueprintPure, Category = "Facing")
	EFacingMode GetCurrentFacingMode() const { return CurrentFacingMode; }

	/** 获取当前注视目标 */
	UFUNCTION(BlueprintPure, Category = "Facing")
	AActor* GetAimTarget() const { return AimTargetActor.Get(); }


	/**
	 * 获取相对于正面的4方向移动方向
	 * 以角色面朝方向为正面（Y轴正方向），速度方向相对于正面划分4个区域
	 * @return 4方向枚举
	 */
	UFUNCTION(BlueprintPure, Category = "Facing")
	E4Direction GetMovementDirection4() const;

	/** 朝向模式改变事件（供外部监听，如 PlayerCharacter 切换速度） */
	UPROPERTY(BlueprintAssignable, Category = "Facing")
	FOnFacingModeChangedDelegate OnFacingModeChanged;

protected:
	virtual void BeginPlay() override;

	/** 当前朝向模式 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facing")
	EFacingMode CurrentFacingMode = EFacingMode::Walking;

	/** 注视目标 */
	UPROPERTY()
	TWeakObjectPtr<AActor> AimTargetActor = nullptr;

	/** 注视旋转速度（度/秒） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Facing", meta = (ClampMin = "90.0"))
	float RotationSpeed = 720.0f;
};