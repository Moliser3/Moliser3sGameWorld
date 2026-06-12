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
}

void AWorldPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bPendingAttack)
    {
        return;
    }

    APlayerCharacter* MyCharacter = Cast<APlayerCharacter>(GetPawn());
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
			if (Elapsed >= 0.0f && Elapsed < CurSkill->InterruptibleAt)
			{
				UE_LOG(LogTemp, Warning, TEXT("[WorldPC] Input ignored — jump flight phase (%.2f < InterruptibleAt=%.2f)"),
					Elapsed, CurSkill->InterruptibleAt);
				bPendingAttack = false;
				return;
			}
			// 收尾阶段（可打断）：放行
		}
	}

	// 执行点击检测
	FClickHitResult ClickResult = ClickDetectionComponent->DetectMouseClick(false);
	if (!ClickResult.bHitSuccess)
	{
		return;
	}

	// 存储点击位置（供跳跃技能读取目标位置）
	LastClickTarget = ClickResult.HitLocation;

	// ── 统一处理：先执行技能队列的下一个技能 ──
	// 跳跃就是技能，不应该特殊判断。
	// ActivateNextSkill 内部会处理：
	//   1. 收尾阶段打断当前技能 → 执行队列中的下一个（跳跃/出拳）
	//   2. IDLE 状态 → 从队列头部执行
	//   3. 飞行阶段不可打断 → 忽略
	//   4. 队列空 → 自动填充
	if (SkillSys)
	{
		// 如果是点击敌人且 MaxAttackRange > 0 且距离超远时，需要先移动到攻击距离
		// 否则直接执行技能
		AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get());
		if (ClickedEnemy)
		{
			float Dist = FVector::Dist(MyCharacter->GetActorLocation(), ClickedEnemy->GetActorLocation());
			float MaxRange = SkillSys->GetMaxAttackRange();

			if (MaxRange > 0 && Dist > MaxRange && !SkillSys->IsNextSkillMovement())
			{
				// 远距敌人 + 下一个技能不是移动技能 → 移动到攻击距离
				FVector DirToEnemy = (ClickedEnemy->GetActorLocation() - MyCharacter->GetActorLocation()).GetSafeNormal2D();
				FVector MoveDest = ClickedEnemy->GetActorLocation() - DirToEnemy * MaxRange;
				MyCharacter->MoveToLocation(MoveDest);

				if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
				{
					FacingComp->SetAimTarget(ClickedEnemy);
				}
				bPendingAttack = true;
				PendingMaxRange = MaxRange;
				return;
			}

			// 近距敌人或下一个技能是移动技能 — 设置注视后执行技能
			if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
			{
				FacingComp->SetAimTarget(ClickedEnemy);
			}
			LastClickTarget = ClickedEnemy->GetActorLocation();
		}

		// 执行下一个技能（打断/IDLE/COMBO 都能处理）
		bPendingAttack = false;
		SkillSys->ActivateNextSkill();
		return;
	}

	// 没有技能系统 → 直接移动
	MyCharacter->MoveToLocation(ClickResult.HitLocation);
}
