// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClickDetectionComponent.generated.h"

/**
 * 鼠标点击检测结果
 */
USTRUCT(BlueprintType)
struct FClickHitResult
{
    GENERATED_BODY()

    /** 是否点击到了有效对象 */
    UPROPERTY(BlueprintReadOnly, Category = "Click")
    bool bHitSuccess = false;

    /** 点击位置的世界坐标 */
    UPROPERTY(BlueprintReadOnly, Category = "Click")
    FVector HitLocation = FVector::ZeroVector;

    /** 点击到的 Actor（如果有） */
    UPROPERTY(BlueprintReadOnly, Category = "Click")
    TWeakObjectPtr<AActor> HitActor = nullptr;

    /** 点击到的组件（如果有） */
    UPROPERTY(BlueprintReadOnly, Category = "Click")
    TWeakObjectPtr<UPrimitiveComponent> HitComponent = nullptr;
};

/** 点击按钮类型 */
UENUM(BlueprintType)
enum class EClickButton : uint8
{
    Left   UMETA(DisplayName = "左键"),
    Right  UMETA(DisplayName = "右键")
};

/** 点击事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClickedDelegate, EClickButton, Button, const FClickHitResult&, HitResult);

/**
 * 鼠标点击检测组件
 * 附加到 PlayerController 上，处理屏幕点击检测
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UClickDetectionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UClickDetectionComponent();

protected:
    virtual void BeginPlay() override;

public:
    /**
     * 检测鼠标点击位置，返回点击到的结果
     * @param bDrawDebug 是否绘制调试线
     * @return 点击检测结果结构体
     */
    UFUNCTION(BlueprintCallable, Category = "Click")
    FClickHitResult DetectMouseClick(bool bDrawDebug = false) const;

    /**
     * 主动执行一次点击检测（由外部触发，如 PlayerController 的输入绑定）
     * @param Button 点击的按钮类型（左键/右键）
     * @param bDrawDebug 是否绘制调试线
     */
    UFUNCTION(BlueprintCallable, Category = "Click")
    void PerformClick(EClickButton Button, bool bDrawDebug = false);

public:
    /** 鼠标点击检测的碰撞通道 */
    UPROPERTY(EditDefaultsOnly, Category = "Click")
    TEnumAsByte<ECollisionChannel> ClickTraceChannel = ECC_Visibility;

    /** 点击事件（左键/右键触发时广播） */
    UPROPERTY(BlueprintAssignable, Category = "Click")
    FOnClickedDelegate OnClicked;

    /** 仅左键点击事件 */
    UPROPERTY(BlueprintAssignable, Category = "Click")
    FOnClickedDelegate OnLeftClicked;

    /** 仅右键点击事件 */
    UPROPERTY(BlueprintAssignable, Category = "Click")
    FOnClickedDelegate OnRightClicked;
};