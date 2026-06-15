// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldPlayerController.h"
#include "Component/Input/ClickDetectionComponent.h"
#include "Component/Camera/CameraControllerComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"
#include "Skill/SkillBase.h"
#include "Animation/AnimInstance.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "Engine/World.h"

AWorldPlayerController::AWorldPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    DefaultMouseCursor = EMouseCursor::Default;

    ClickDetectionComponent = CreateDefaultSubobject<UClickDetectionComponent>(TEXT("ClickDetectionComponent"));
    CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>(TEXT("CameraControllerComponent"));

    PrimaryActorTick.bCanEverTick = true;
}

void AWorldPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
    if (!MyCharacter) return;

    UFacingComponent* FacingComp = MyCharacter->GetFacingComponent();
    USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
    if (!FacingComp || !SkillSys) return;

    AActor* AimTarget = FacingComp->GetAimTarget();

    // ── 1. 战斗状态转移：超出战斗感知范围或无目标 → 默认状态 ──
    if (CurrentPlayerState == EPlayerState::Battle)
    {
        if (!AimTarget)
        {
            SetPlayerState(EPlayerState::Default);
        }
        else
        {
            float Dist = FVector::Dist(MyCharacter->GetActorLocation(), AimTarget->GetActorLocation());
            if (Dist > MyCharacter->GetBattlePerceptionRange())
            {
                SetPlayerState(EPlayerState::Default);
            }
        }
    }

    // ── 2. 技能结束恢复注视（位移技能释放完毕后切回）──
    ESkillPhase CurrentPhase = SkillSys->GetSkillPhase();
    if (CurrentPlayerState == EPlayerState::Battle && bPreviousSkillActive && CurrentPhase == ESkillPhase::Idle)
    {
        AActor* SkillEndTarget = FacingComp->GetAimTarget();
        if (SkillEndTarget && FacingComp->GetCurrentFacingMode() != EFacingMode::Aiming)
        {
            float Dist = FVector::Dist(MyCharacter->GetActorLocation(), SkillEndTarget->GetActorLocation());
            if (Dist <= MyCharacter->GetBattlePerceptionRange())
            {
                FacingComp->SetAimTarget(SkillEndTarget);
            }
        }
    }
    bPreviousSkillActive = (CurrentPhase != ESkillPhase::Idle);

    // ── 3. Shift奔跑结束后的注视恢复 ──
    if (CurrentPlayerState == EPlayerState::Battle && bPendingRestoreAiming)
    {
        AActor* RunTarget = FacingComp->GetAimTarget();
        if (!RunTarget)
        {
            bPendingRestoreAiming = false;
        }
        else if (MyCharacter->GetVelocity().IsNearlyZero())
        {
            bPendingRestoreAiming = false;
            float Dist = FVector::Dist(MyCharacter->GetActorLocation(), RunTarget->GetActorLocation());
            if (Dist <= MyCharacter->GetBattlePerceptionRange())
            {
                if (FacingComp->GetCurrentFacingMode() != EFacingMode::Aiming)
                {
                    FacingComp->SetAimTarget(RunTarget);
                }
            }
        }
    }

    // ── 4. 自动攻击（移动到技能范围后释放技能）──
    if (!bPendingAttack) return;

    if (!AimTarget)
    {
        bPendingAttack = false;
        return;
    }

    float Dist = FVector::Dist(MyCharacter->GetActorLocation(), AimTarget->GetActorLocation());
    if (Dist <= PendingMaxRange + 80.0f)
    {
        bPendingAttack = false;
        SkillSys->ActivateNextSkill();
    }
}

void AWorldPlayerController::SetPlayerState(EPlayerState NewState)
{
    if (CurrentPlayerState == NewState) return;

    APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
    EPlayerState OldState = CurrentPlayerState;
    CurrentPlayerState = NewState;

    if (NewState == EPlayerState::Default)
    {
        // 退出战斗状态：清除注视目标、取消待攻击与奔跑恢复标志
        bPendingAttack = false;
        bPendingRestoreAiming = false;
        if (MyCharacter)
        {
            if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
            {
                FacingComp->ClearAimTarget();
            }
        }
    }
}

void AWorldPlayerController::BeginPlay()
{
    Super::BeginPlay();
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

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
    // 左键已废弃
}

void AWorldPlayerController::OnRightMouseClick()
{
    if (!ClickDetectionComponent) return;

    APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
    if (!MyCharacter) return;

    USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
    UFacingComponent* FacingComp = MyCharacter->GetFacingComponent();
    UCharacterMovementComponent* MoveComp = MyCharacter->GetCharacterMovement();
    if (!SkillSys || !FacingComp || !MoveComp) return;

    // Shift键检测（奔跑）
    bool bShiftDown = IsInputKeyDown(EKeys::LeftShift);

    // 执行点击检测
    FClickHitResult ClickResult = ClickDetectionComponent->DetectMouseClick(false);

    // 更新 LastClickTarget
    if (ClickResult.bHitSuccess)
    {
        LastClickTarget = ClickResult.HitLocation;
    }

    // ── 处理点击敌人 ──
    AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get());
    if (ClickedEnemy)
    {
        // 锁定敌人 → 进入战斗状态
        FacingComp->SetAimTarget(ClickedEnemy);
        SetPlayerState(EPlayerState::Battle);
        LastClickTarget = ClickedEnemy->GetActorLocation();

        // 设置移动速度
        MoveComp->MaxWalkSpeed = bShiftDown ? MyCharacter->GetRunSpeed() : MyCharacter->GetWalkSpeed();

        float MaxRange = SkillSys->GetMaxSkillRange();
        ESkillCategory NextCat = SkillSys->GetNextSkillCategory();

        // 位移/辅助技能 → 直接执行（不受距离约束）
        if (NextCat == ESkillCategory::Movement || NextCat == ESkillCategory::Utility)
        {
            bPendingAttack = false;
            SkillSys->ActivateNextSkill();
            return;
        }

        // 非移动技能：检查距离
        float Dist = FVector::Dist(MyCharacter->GetActorLocation(), ClickedEnemy->GetActorLocation());
        if (MaxRange > 0 && Dist > MaxRange)
        {
            // 超出攻击范围 → 走近后自动攻击
            bPendingAttack = true;
            PendingMaxRange = MaxRange;
            if (UAnimInstance* AnimInst = MyCharacter->GetMesh()->GetAnimInstance())
            {
                AnimInst->Montage_Stop(0.1f);
            }
            MyCharacter->MoveToLocation(ClickedEnemy->GetActorLocation());
            return;
        }

        // 在范围内 → 直接攻击
        bPendingAttack = false;
        SkillSys->ActivateNextSkill();
        return;
    }

    // ── 点击地面 ──
    if (ClickResult.bHitSuccess)
    {
        ESkillCategory NextCat = SkillSys->GetNextSkillCategory();
        bool bIsMovementSkill = (NextCat == ESkillCategory::Movement ||
                                 NextCat == ESkillCategory::Hybrid ||
                                 NextCat == ESkillCategory::Utility);

        // 位移/辅助/复合技能 → 直接执行
        if (bIsMovementSkill)
        {
            bPendingAttack = false;
            SkillSys->ActivateNextSkill();
            return;
        }

        // ── 非技能移动 ──
        if (CurrentPlayerState == EPlayerState::Battle)
        {
            if (bShiftDown)
            {
                // 战斗状态 + Shift奔跑：临时切为行走模式，面朝移动方向
                MoveComp->bOrientRotationToMovement = true;
                MoveComp->MaxWalkSpeed = MyCharacter->GetRunSpeed();
                bPendingRestoreAiming = true;
            }
            else
            {
                // 战斗状态 + 普通行走：保持注视模式，面朝敌人
                MoveComp->MaxWalkSpeed = MyCharacter->GetWalkSpeed();
            }
        }
        else
        {
            // 默认状态
            MoveComp->MaxWalkSpeed = bShiftDown ? MyCharacter->GetRunSpeed() : MyCharacter->GetWalkSpeed();
        }

        if (UAnimInstance* AnimInst = MyCharacter->GetMesh()->GetAnimInstance())
        {
            AnimInst->Montage_Stop(0.1f);
        }
        MyCharacter->MoveToLocation(ClickResult.HitLocation);
    }
}