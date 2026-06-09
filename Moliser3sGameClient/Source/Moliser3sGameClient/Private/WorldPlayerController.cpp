// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldPlayerController.h"
#include "Component/Input/ClickDetectionComponent.h"
#include "EnhancedInputSubsystems.h"
#include "BaseCharacter.h"

AWorldPlayerController::AWorldPlayerController()
{
    // 默认启用鼠标光标
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    // 默认光标样式
    DefaultMouseCursor = EMouseCursor::Default;

    // 创建点击检测组件
    ClickDetectionComponent = CreateDefaultSubobject<UClickDetectionComponent>(TEXT("ClickDetectionComponent"));
}

void AWorldPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 设置输入模式为仅游戏响应鼠标，同时UI也能响应
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    // 自动注册默认 IMC
    if (DefaultMappingContext)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AWorldPlayerController::OnLeftMouseClick()
{
    if (ClickDetectionComponent)
    {
        ClickDetectionComponent->PerformClick(EClickButton::Left, false);
    }
}

void AWorldPlayerController::OnRightMouseClick()
{
    if (!ClickDetectionComponent)
    {
        return;
    }

    // 执行点击检测
    FClickHitResult ClickResult = ClickDetectionComponent->DetectMouseClick(false);

    // 只有点击到有效位置时才移动
    if (ClickResult.bHitSuccess)
    {
        if (ABaseCharacter* MyCharacter = Cast<ABaseCharacter>(GetPawn()))
        {
            MyCharacter->MoveToLocation(ClickResult.HitLocation);
        }
    }
}
