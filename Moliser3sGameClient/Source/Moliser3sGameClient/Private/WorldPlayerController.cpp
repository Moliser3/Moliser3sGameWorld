// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldPlayerController.h"
#include "Component/Input/ClickDetectionComponent.h"
#include "Component/Camera/CameraControllerComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Skill/SkillSystemComponent.h"
#include "Skill/SkillBase.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
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

    // 创建摄像机控制器组件
    CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>(TEXT("CameraControllerComponent"));

    // 开启 Tick 以检测自动攻击
    PrimaryActorTick.bCanEverTick = true;
    // v2: force recompile - ensure !NextSkill guard branch is active
}

void AWorldPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ── 调试：显示跳跃技能后摇倒计时 ──
    APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
    if (MyCharacter)
    {
        if (USkillSystemComponent* SS = MyCharacter->GetSkillSystem())
        {
            if (SS->IsSkillActive())
            {
                USkillBase* CurSkill = SS->GetCurrentSkill();
                if (CurSkill && CurSkill->bIsMovementSkill)
                {
                    float Elapsed = SS->GetCurrentSkillElapsed();
                    float Remaining = CurSkill->Duration - Elapsed;
                    float EffectiveInterruptAt = CurSkill->GetInterruptibleAt();
                    if (Remaining > 0 && Elapsed >= EffectiveInterruptAt)
                    {
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow,
                                FString::Printf(TEXT("后摇 %.2fs / 剩余 %.2fs  [可打断]"), Elapsed, Remaining));
                        }
                    }
                    else if (Remaining > 0)
                    {
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Red,
                                FString::Printf(TEXT("飞行中 %.2fs / %.2fs  [不可打断]"), Elapsed, CurSkill->Duration));
                        }
                    }
                }
            }
            else
            {
                // 技能不在激活状态时清除倒计时显示
                if (GEngine)
                {
                    GEngine->ClearOnScreenDebugMessages();
                }
            }
        }
    }

    if (!bPendingAttack)
    {
        return;
    }

    MyCharacter = Cast<APlayerCharacter>(GetPawn());
    if (!MyCharacter)
    {
        bPendingAttack = false;
        return;
    }

    // 检测是否在注视模式下（说明有目标）
    UFacingComponent* FacingComp = MyCharacter->GetFacingComponent();
    if (!FacingComp || FacingComp->GetCurrentFacingMode() != EFacingMode::Aiming)
    {
        bPendingAttack = false;
        return;
    }

    AActor* AimTarget = FacingComp->GetAimTarget();
    if (!AimTarget)
    {
        bPendingAttack = false;
        return;
    }

    // 计算到目标的距离
    float Dist = FVector::Dist(MyCharacter->GetActorLocation(), AimTarget->GetActorLocation());

    // 判断是否停止移动（速度接近零 = 到达目的地）
    UCharacterMovementComponent* MoveComp = MyCharacter->GetCharacterMovement();
    bool bStopped = MoveComp && MoveComp->Velocity.SizeSquared2D() < 100.0f; // 速度 < 10cm/s

    UE_LOG(LogTemp, Warning, TEXT("[AutoAttack] Dist=%.0f MaxRange=%.0f Stopped=%d Active=%d"),
        Dist, PendingMaxRange, bStopped, MyCharacter->GetSkillSystem() ? MyCharacter->GetSkillSystem()->IsSkillActive() : -1);

    // 到达攻击范围 或 角色已停止移动 → 触发攻击
    // 直接调用 ActivateNextSkill，由它内部判断可打断还是忽略
    bool bInRange = Dist <= PendingMaxRange + 80.0f;
    if (bInRange || bStopped)
    {
        USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
        if (SkillSys)
        {
            bPendingAttack = false;
            UE_LOG(LogTemp, Warning, TEXT("[AutoAttack] Triggering attack!"));
            SkillSys->ActivateNextSkill();
        }
    }
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
    // 左键暂时保留（当前未使用），可由蓝图解除绑定
}

void AWorldPlayerController::OnRightMouseClick()
{
	if (!ClickDetectionComponent)
	{
		return;
	}

	APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
	if (!MyCharacter)
	{
		return;
	}

	USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();

	// ── 跳跃飞行阶段拦截 ──
	// 移动技能在不可打断阶段（飞行中），忽略所有输入
	if (SkillSys && SkillSys->IsSkillActive())
	{
		USkillBase* CurSkill = SkillSys->GetCurrentSkill();
		if (CurSkill && CurSkill->bIsMovementSkill)
		{
			float Elapsed = SkillSys->GetCurrentSkillElapsed();
			float EffectiveInterruptAt = CurSkill->GetInterruptibleAt();
			if (Elapsed >= 0.0f && Elapsed < EffectiveInterruptAt)
			{
				UE_LOG(LogTemp, Warning, TEXT("[WorldPC] Input ignored — jump flight phase (%.2f < InterruptibleAt=%.2f)"),
					Elapsed, EffectiveInterruptAt);
				bPendingAttack = false;
				return;
			}
			// 收尾阶段（可打断）：放行
		}
	}

	// 执行点击检测（用于获取点击位置和敌人目标）
	FClickHitResult ClickResult = ClickDetectionComponent->DetectMouseClick(false);

	// ── 调试屏幕信息：点击检测结果 ──
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 2.0f, ClickResult.bHitSuccess ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("点击检测: %s  位置: %s"),
				ClickResult.bHitSuccess ? TEXT("命中") : TEXT("未命中"),
				*ClickResult.HitLocation.ToString()));
		if (ClickResult.bHitSuccess)
		{
			GEngine->AddOnScreenDebugMessage(3, 2.0f, FColor::Cyan,
				FString::Printf(TEXT("点击目标: %s"),
					ClickResult.HitActor.IsValid() ? *ClickResult.HitActor->GetName() : TEXT("地面")));
		}
	}

	// 如果点击检测成功，更新目标位置和注视对象
	if (ClickResult.bHitSuccess)
	{
		LastClickTarget = ClickResult.HitLocation;

		AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get());
		if (ClickedEnemy)
		{
			if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
				FacingComp->SetAimTarget(ClickedEnemy);
			LastClickTarget = ClickedEnemy->GetActorLocation();
		}
	}
	// ── 点击检测失败时显示当前 LastClickTarget ──
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(4, 2.0f, FColor::Orange,
			FString::Printf(TEXT("LastClickTarget(旧): %s"), *LastClickTarget.ToString()));
	}

	// ── 无论点击检测是否成功，都尝试执行技能 ──
	// ActivateNextSkill 内部包含打断/连招/IDLE 状态机逻辑
	// 这样在收尾期点击地面即使没有碰撞体命中，也能触发打断执行下一个技能
	if (SkillSys)
	{
		bPendingAttack = false;
		SkillSys->ActivateNextSkill();
		return;
	}

	// 没有技能系统且点击检测成功 → 直接移动
	if (ClickResult.bHitSuccess)
	{
		MyCharacter->MoveToLocation(ClickResult.HitLocation);
	}
}
