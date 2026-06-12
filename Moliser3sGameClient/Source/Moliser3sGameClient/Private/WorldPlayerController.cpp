// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldPlayerController.h"
#include "Component/Input/ClickDetectionComponent.h"
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

	// 存储点击位置（供技能系统使用，如跳跃技能读取此位置作为目标）
	LastClickTarget = ClickResult.HitLocation;

	// ── 第一步：检查下一个技能是否为移动技能（如跳跃） ──
	// 移动技能不受攻击距离/敌人判断约束，点击即触发
	USkillSystemComponent* SkillSys = MyCharacter->GetSkillSystem();
	if (SkillSys && SkillSys->IsNextSkillMovement())
	{
		SkillSys->ActivateNextSkill();
		return;
	}

	// ── 第二步：非移动技能 → 原有攻击/移动逻辑 ──

	// 右键点击到敌人
	if (AEnemyCharacter* ClickedEnemy = Cast<AEnemyCharacter>(ClickResult.HitActor.Get()))
	{
		float Dist = FVector::Dist(MyCharacter->GetActorLocation(), ClickedEnemy->GetActorLocation());

		// 获取技能队列的 MaxAttackRange，-1 表示全远程技能
		float MaxRange = SkillSys ? SkillSys->GetMaxAttackRange() : -1.0f;

		if (MaxRange > 0 && Dist <= MaxRange)
		{
			// 距离 ≤ 最大攻击距离 → 直接攻击（内部打断逻辑由 ActivateNextSkill 处理）
			if (SkillSys)
			{
				SkillSys->ActivateNextSkill();
			}
		}
		else if (MaxRange > 0)
		{
			// 距离 > 最大攻击距离 → 移动到最大攻击距离处 + 进入注视模式
			if (UFacingComponent* FacingComp = MyCharacter->GetFacingComponent())
			{
				FVector DirToEnemy = (ClickedEnemy->GetActorLocation() - MyCharacter->GetActorLocation()).GetSafeNormal2D();
				FVector MoveDest = ClickedEnemy->GetActorLocation() - DirToEnemy * MaxRange;
				MyCharacter->MoveToLocation(MoveDest);
				FacingComp->SetAimTarget(ClickedEnemy);

				// 设置待攻击标记：走到攻击距离后自动释放技能
				bPendingAttack = true;
				PendingMaxRange = MaxRange;
				UE_LOG(LogTemp, Warning, TEXT("[AutoAttack] Moving to attack range, Dist=%.0f MaxRange=%.0f"), Dist, MaxRange);
			}
		}
		else
		{
			// MaxAttackRange = -1（全远程技能）→ 直接攻击（内部打断逻辑由 ActivateNextSkill 处理）
			if (SkillSys)
			{
				SkillSys->ActivateNextSkill();
			}
		}
		return;
	}

	// 右键点击到地面或其他位置 → 移动（保持当前注视模式）
	MyCharacter->MoveToLocation(ClickResult.HitLocation);
}
