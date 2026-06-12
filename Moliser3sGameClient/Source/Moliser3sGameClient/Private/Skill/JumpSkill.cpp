// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/JumpSkill.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "WorldPlayerController.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

void UJumpSkill::Execute(AActor* Instigator)
{
	if (!Instigator)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar)
	{
		return;
	}

	// 如果角色正在跳跃中或在空中，不重复触发
	if (bIsJumping || (OwnerChar->GetCharacterMovement() && OwnerChar->GetCharacterMovement()->IsFalling()))
	{
		return;
	}

	// 获取角色当前位置作为起点
	JumpStartLoc = OwnerChar->GetActorLocation();

	// 从 Controller 获取右键点击的目标位置
	AController* Ctl = OwnerChar->GetController();
	AWorldPlayerController* PlayerCtl = Cast<AWorldPlayerController>(Ctl);
	if (!PlayerCtl)
	{
		return;
	}

	FVector ClickTarget = PlayerCtl->GetLastClickTarget();

	// 计算水平方向距离
	FVector Start2D = JumpStartLoc;
	Start2D.Z = 0;
	FVector Target2D = ClickTarget;
	Target2D.Z = 0;

	float HorizontalDist = FVector::Dist(Start2D, Target2D);

	// 如果目标太远，限制到 JumpRange
	if (HorizontalDist > JumpRange)
	{
		FVector Dir = (Target2D - Start2D).GetSafeNormal();
		Target2D = Start2D + Dir * JumpRange;
	}

	// 检测目标是否可达（NavMesh）
	FVector FinalTarget = Target2D;
	FinalTarget.Z = ClickTarget.Z; // 保留原始高度

	if (!IsTargetReachable(Instigator, FinalTarget))
	{
		// 不可达 → 原地起跳（目标 = 起点，仅垂直抛物线）
		JumpTargetLoc = JumpStartLoc;
		UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Target unreachable, jumping in place"));
	}
	else
	{
		JumpTargetLoc = FinalTarget;
		// 朝向目标方向
		FVector DirToTarget = (JumpTargetLoc - JumpStartLoc).GetSafeNormal2D();
		if (!DirToTarget.IsNearlyZero())
		{
			OwnerChar->SetActorRotation(DirToTarget.Rotation());
		}
		UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Jumping to target (%.0f cm away)"), HorizontalDist);
	}

	// 技能释放时停止移动
	if (Ctl)
	{
		Ctl->StopMovement();
	}

	// 播放跳跃蒙太奇（如果已配置）
	PlaySkillMontage(Instigator);

	// 初始化跳跃状态
	bIsJumping = true;
	JumpProgress = 0.0f;

	// 设置 Timer 驱动跳跃（每 0.016s ≈ 60fps）
	float TickInterval = 0.016f;
	Instigator->GetWorld()->GetTimerManager().SetTimer(JumpTimerHandle, 
		FTimerDelegate::CreateUObject(this, &UJumpSkill::OnJumpTick, Instigator),
		TickInterval, true);
}

void UJumpSkill::OnJumpTick(AActor* Instigator)
{
	if (!Instigator || !bIsJumping)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar)
	{
		EndJump(Instigator);
		return;
	}

	// 推进进度
	float DeltaTime = 0.016f; // 匹配 Timer 间隔
	JumpProgress += DeltaTime / FlyDuration;

	// 限制进度不超过 1.0
	if (JumpProgress > 1.0f)
	{
		JumpProgress = 1.0f;
	}

	// 计算水平位置（线性插值）
	FVector HorizontalPos = FMath::Lerp(JumpStartLoc, JumpTargetLoc, JumpProgress);
	HorizontalPos.Z = JumpStartLoc.Z; // 水平位置保持起点高度

	// 计算垂直偏移：抛物线公式 4 * h * t * (1-t)
	float VerticalOffset = 4.0f * JumpHeight * JumpProgress * (1.0f - JumpProgress);

	FVector NewLocation = HorizontalPos;
	NewLocation.Z += VerticalOffset;

	// 碰撞检测：检查新位置是否有阻挡
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Instigator);

	FCollisionShape CheckShape = FCollisionShape::MakeSphere(30.0f); // 角色半径
	bool bBlocked = Instigator->GetWorld()->OverlapBlockingTestByChannel(
		NewLocation,
		FQuat::Identity,
		ECC_WorldStatic,
		CheckShape,
		QueryParams
	);

	if (bBlocked)
	{
		// 撞到障碍物 → 落地
		UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Hit obstacle, landing"));
		EndJump(Instigator);
		return;
	}

	// 设置角色位置
	OwnerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

	// 到达终点
	if (JumpProgress >= 1.0f)
	{
		EndJump(Instigator);
	}
}

void UJumpSkill::EndJump(AActor* Instigator)
{
	if (!bIsJumping)
	{
		return;
	}

	bIsJumping = false;
	JumpProgress = 0.0f;

	// 停止 Timer
	if (Instigator && Instigator->GetWorld())
	{
		Instigator->GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Jump ended"));
}

bool UJumpSkill::IsTargetReachable(AActor* Instigator, const FVector& Target) const
{
	if (!Instigator)
	{
		return false;
	}

	// 使用 NavigationSystem 检测目标点是否在 NavMesh 上
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Instigator->GetWorld());
	if (!NavSys)
	{
		// 没有导航系统则默认可达
		return true;
	}

	FNavLocation NavLocation;
	bool bReachable = NavSys->ProjectPointToNavigation(Target, NavLocation, FVector(50.0f));

	return bReachable;
}