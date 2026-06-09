// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Input/ClickDetectionComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"

UClickDetectionComponent::UClickDetectionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UClickDetectionComponent::BeginPlay()
{
    Super::BeginPlay();
}

FClickHitResult UClickDetectionComponent::DetectMouseClick(bool bDrawDebug) const
{
    FClickHitResult Result;

    // 获取所属 PlayerController
    APlayerController* OwnerController = Cast<APlayerController>(GetOwner());
    if (!OwnerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("ClickDetectionComponent: Owner is not a PlayerController!"));
        return Result;
    }

    // 获取鼠标位置
    float MouseX = 0.0f, MouseY = 0.0f;
    if (!OwnerController->GetMousePosition(MouseX, MouseY))
    {
        // 无法获取鼠标位置（例如在UI上）
        return Result;
    }

    // 从鼠标位置发出射线
    FVector WorldOrigin;
    FVector WorldDirection;
    if (!OwnerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldOrigin, WorldDirection))
    {
        // 无法将屏幕坐标转换为世界坐标
        return Result;
    }

    // 进行射线检测
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = false;

    // 忽略自己
    if (OwnerController->GetPawn())
    {
        QueryParams.AddIgnoredActor(OwnerController->GetPawn());
    }

    const float TraceDistance = 100000.0f; // 1000米检测距离
    FVector TraceEnd = WorldOrigin + (WorldDirection * TraceDistance);

    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        WorldOrigin,
        TraceEnd,
        ClickTraceChannel,
        QueryParams
    );

    // 填充结果
    Result.bHitSuccess = bHit;
    if (bHit)
    {
        Result.HitLocation = HitResult.Location;
        Result.HitActor = HitResult.GetActor();
        Result.HitComponent = HitResult.GetComponent();

        // 调试绘制
        if (bDrawDebug)
        {
            DrawDebugLine(GetWorld(), WorldOrigin, HitResult.Location, FColor::Green, false, 2.0f, 0, 1.0f);
            DrawDebugSphere(GetWorld(), HitResult.Location, 8.0f, 12, FColor::Yellow, false, 2.0f);
            if (HitResult.GetActor())
            {
                DrawDebugBox(GetWorld(), HitResult.GetActor()->GetActorLocation(), FVector(50.0f), FColor::Red, false, 2.0f);
            }
        }
    }
    else if (bDrawDebug)
    {
        // 没有命中时绘制红色射线
        DrawDebugLine(GetWorld(), WorldOrigin, TraceEnd, FColor::Red, false, 2.0f, 0, 1.0f);
    }

    return Result;
}

void UClickDetectionComponent::PerformClick(EClickButton Button, bool bDrawDebug)
{
    FClickHitResult ClickResult = DetectMouseClick(bDrawDebug);

    // 广播事件
    OnClicked.Broadcast(Button, ClickResult);

    switch (Button)
    {
    case EClickButton::Left:
        OnLeftClicked.Broadcast(Button, ClickResult);
        break;

    case EClickButton::Right:
        OnRightClicked.Broadcast(Button, ClickResult);
        break;
    }
}