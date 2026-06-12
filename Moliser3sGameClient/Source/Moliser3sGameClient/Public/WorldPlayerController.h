// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WorldPlayerController.generated.h"

class UClickDetectionComponent;
class UInputMappingContext;
class UCameraControllerComponent;

/**
 * 
 */
UCLASS()
class MOLISER3SGAMECLIENT_API AWorldPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AWorldPlayerController();

    virtual void Tick(float DeltaTime) override;

    /** 获取点击检测组件 */
    UFUNCTION(BlueprintCallable, Category = "Click")
    UClickDetectionComponent* GetClickDetectionComponent() const { return ClickDetectionComponent; }

    /** 获取摄像机控制器组件 */
    UFUNCTION(BlueprintPure, Category = "Camera")
    UCameraControllerComponent* GetCameraController() const { return CameraControllerComponent; }

    /** 处理鼠标左键点击 - 由蓝图输入事件调用 */
    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnLeftMouseClick();

    /** 处理鼠标右键点击 - 由蓝图输入事件调用 */
    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnRightMouseClick();

    /** 获取最后一次右键点击的目标位置（供技能系统使用） */
    UFUNCTION(BlueprintPure, Category = "Click")
    FVector GetLastClickTarget() const { return LastClickTarget; }

    /** 设置最后一次右键点击的目标位置（供技能系统修改，如跳跃截断后同步） */
    UFUNCTION(BlueprintCallable, Category = "Click")
    void SetLastClickTarget(const FVector& NewTarget) { LastClickTarget = NewTarget; }

protected:
    virtual void BeginPlay() override;

    /** 默认输入映射上下文 - 在蓝图中赋值 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

protected:
    /** 点击检测组件 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Click")
    TObjectPtr<UClickDetectionComponent> ClickDetectionComponent;

    /** 摄像机控制器组件 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraControllerComponent> CameraControllerComponent;

    /** 最后一次右键点击的目标位置 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Click")
    FVector LastClickTarget = FVector::ZeroVector;

    /** 是否等待移动到攻击距离后自动攻击 */
    bool bPendingAttack = false;

    /** 待攻击的最大距离目标参数 */
    float PendingMaxRange = 0.0f;
};
