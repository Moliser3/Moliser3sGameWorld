// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldPlayerController.h"
#include "Component/Input/ClickDetectionComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "Engine/World.h"

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
    // 左键释放技能循环队列中的下一个技能（系统内部自行处理 Duration 冷却）
    if (APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn()))
    {
        if (USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem())
        {
            SkillSys->ActivateNextSkill();
        }
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

    if (!ClickResult.bHitSuccess)
    {
        return;
    }

    APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
    if (!MyCharacter)
    {
        return;
    }

    // 右键点击到敌人 → 进入注视模式并移动到停止距离
    if (AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get()))
    {
        if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
        {
            // 如果距离大于停止距离，移动到敌人旁边的停止距离位置
            float Dist = FVector::Dist(MyCharacter->GetActorLocation(), ClickedEnemy->GetActorLocation());
            float StopDist = MyCharacter->GetStopDistance();
            if (Dist > StopDist)
            {
                FVector DirToEnemy = (ClickedEnemy->GetActorLocation() - MyCharacter->GetActorLocation()).GetSafeNormal2D();
                FVector MoveDest = ClickedEnemy->GetActorLocation() - DirToEnemy * StopDist;
                MyCharacter->MoveToLocation(MoveDest);
            }
            FacingComp->SetAimTarget(ClickedEnemy);
        }
        return;
    }

    // 右键点击到地面或其他位置 → 移动（保持当前注视模式）
    // 只有超出最大距离时才由 FacingComponent::Tick 自动切回行走模式
    MyCharacter->MoveToLocation(ClickResult.HitLocation);
}
