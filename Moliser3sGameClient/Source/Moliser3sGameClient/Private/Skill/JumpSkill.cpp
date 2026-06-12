// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/JumpSkill.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "WorldPlayerController.h"
#include "Component/Skill/SkillSystemComponent.h"
#include "Component/Facing/FacingComponent.h"
#include "Component/Camera/CameraControllerComponent.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UJumpSkill::UJumpSkill()
{
	// 跳跃是移动技能，不受攻击距离/敌人判断约束
	bIsMovementSkill = true;

	// 两阶段跳跃：0.72s 抛物线 + 收尾
	Duration = 2.07f;
	DamageAt = 0.0f; // 跳跃不造成伤害
	InterruptibleAt = 0.72f; // 飞行中不可打断，落地收尾阶段可打断
	MaxAttackRange = -1; // 跳跃不受攻击距离限制
	FlyDuration = 0.72f;
}

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

	// 强制确保飞行阶段不可打断（防止蓝图中 InterruptibleAt 被错误覆盖）
	InterruptibleAt = FMath::Max(InterruptibleAt, FlyDuration);

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

	UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] EXECUTE: Start=%s ClickTarget=%s HDist=%.0f JumpRange=%.0f"),
		*JumpStartLoc.ToString(), *ClickTarget.ToString(), HorizontalDist, JumpRange);

	// 如果目标太远，限制到 JumpRange
	if (HorizontalDist > JumpRange)
	{
		FVector Dir = (Target2D - Start2D).GetSafeNormal();
		Target2D = Start2D + Dir * JumpRange;
	}

	// 检测目标是否可达（NavMesh）
	FVector FinalTarget = Target2D;
	FinalTarget.Z = ClickTarget.Z;

	// 同步更新 Controller 的 LastClickTarget 为实际跳跃目标位置
	if (PlayerCtl)
	{
		PlayerCtl->SetLastClickTarget(FinalTarget);
	}

	bool bReachable = IsTargetReachable(Instigator, FinalTarget);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(5, 2.0f, bReachable ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("跳跃目标: %s  距离: %.0f  可达: %s"),
				*FinalTarget.ToString(), HorizontalDist, bReachable ? TEXT("是") : TEXT("否")));
	}

	if (!bReachable)
	{
		// 不可达 → 原地起跳
		JumpTargetLoc = JumpStartLoc;
		UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Target unreachable, jumping in place"));
	}
	else
	{
		JumpTargetLoc = FinalTarget;
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

	// 通知摄像机控制器：跳跃开始
	if (PlayerCtl)
	{
		if (UCameraControllerComponent* CamCtrl = PlayerCtl->GetCameraController())
		{
			CamCtrl->SetJumping(true);
		}
	}

	// 播放跳跃蒙太奇
	PlaySkillMontage(Instigator);

	// ── Debug：绘制跳跃路径预览线框 ──
	if (Instigator && Instigator->GetWorld())
	{
		UWorld* World = Instigator->GetWorld();
		const int32 NumSamples = 20;
		FVector PrevPoint = JumpStartLoc;
		PrevPoint.Z = JumpStartLoc.Z; // 水平位置
		for (int32 i = 1; i <= NumSamples; i++)
		{
			float T = (float)i / NumSamples;
			FVector HPos = FMath::Lerp(JumpStartLoc, JumpTargetLoc, T);
			HPos.Z = JumpStartLoc.Z;
			float VOffset = 4.0f * JumpHeight * T * (1.0f - T);
			FVector Point = HPos;
			Point.Z += VOffset;

			DrawDebugLine(World, PrevPoint, Point, FColor::Green, false, FlyDuration, 0, 1.0f);
			DrawDebugPoint(World, Point, 4.0f, FColor::Green, false, FlyDuration);
			PrevPoint = Point;
		}
		// 起点和终点标记
		DrawDebugSphere(World, JumpStartLoc, 15.0f, 8, FColor::Blue, false, FlyDuration);
		DrawDebugSphere(World, JumpTargetLoc, 15.0f, 8, FColor::Red, false, FlyDuration);
	}

	// 初始化跳跃状态
	bIsJumping = true;
	JumpProgress = 0.0f;
}

void UJumpSkill::Update(AActor* Instigator, float DeltaTime)
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

	// ── 阶段1：抛物线位移（0 ~ FlyDuration）──
	if (JumpProgress < 1.0f)
	{
		// 推进跳跃进度（使用引擎帧 DeltaTime，与渲染帧完全同步）
		JumpProgress += DeltaTime / FlyDuration;

		// 限制进度不超过 1.0
		if (JumpProgress > 1.0f)
		{
			JumpProgress = 1.0f;
		}

		// 计算水平位置（线性插值）
		FVector HorizontalPos = FMath::Lerp(JumpStartLoc, JumpTargetLoc, JumpProgress);
		HorizontalPos.Z = JumpStartLoc.Z;

		// 计算垂直偏移：抛物线公式 4 * h * t * (1-t)
		float VerticalOffset = 4.0f * JumpHeight * JumpProgress * (1.0f - JumpProgress);

		FVector NewLocation = HorizontalPos;
		NewLocation.Z += VerticalOffset;

		// 碰撞检测
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Instigator);

		FCollisionShape CheckShape = FCollisionShape::MakeSphere(30.0f);
		bool bBlocked = Instigator->GetWorld()->OverlapBlockingTestByChannel(
			NewLocation,
			FQuat::Identity,
			ECC_WorldStatic,
			CheckShape,
			QueryParams
		);

		if (bBlocked)
		{
			UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Hit obstacle, landing early"));
			JumpProgress = 1.0f;
			// 不 EndJump，继续进入阶段2收尾
			OwnerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			return;
		}

		// 设置角色位置
		OwnerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	// ── 阶段2：落地收尾（FlyDuration ~ Duration）──
	// 抛物线位移已完成，角色不再移动
	// 不清除技能状态，让 SkillSystemComponent 的 Duration 检测自然到期进入 COMBO_WINDOW
	// 这样收尾动画可以完整播放
	if (JumpProgress >= 1.0f && bIsJumping)
	{
		bIsJumping = false;

		// 通知摄像机控制器：跳跃位移结束，恢复 Z 弹性
		if (AController* Ctl = OwnerChar->GetController())
		{
			if (AWorldPlayerController* PlayerCtl = Cast<AWorldPlayerController>(Ctl))
			{
				if (UCameraControllerComponent* CamCtrl = PlayerCtl->GetCameraController())
				{
					CamCtrl->SetJumping(false);
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Movement finished, waiting for Duration (%.2f) to expire for landing anim"), Duration);
	}
}

void UJumpSkill::OnInterrupt(AActor* Instigator)
{
	// 跳跃被打断时清理运行时状态
	// （打断仅在落地收尾阶段的 ActivateNextSkill 中触发，飞行中 InterruptibleAt=0.72 不可打断）
	if (bIsJumping)
	{
		bIsJumping = false;
		JumpProgress = 0.0f;

		UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Interrupted!"));
	}

	// 通知摄像机控制器：跳跃结束
	if (ACharacter* OwnerChar = Cast<ACharacter>(Instigator))
	{
		if (AController* Ctl = OwnerChar->GetController())
		{
			if (AWorldPlayerController* PlayerCtl = Cast<AWorldPlayerController>(Ctl))
			{
				if (UCameraControllerComponent* CamCtrl = PlayerCtl->GetCameraController())
				{
					CamCtrl->SetJumping(false);
				}
			}
		}
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

	if (ACharacter* OwnerChar = Cast<ACharacter>(Instigator))
	{
		if (USkillSystemComponent* SkillSys = OwnerChar->FindComponentByClass<USkillSystemComponent>())
		{
			SkillSys->ForceEndCurrentSkill();
		}

		// 通知摄像机控制器：跳跃结束
		if (AController* Ctl = OwnerChar->GetController())
		{
			if (AWorldPlayerController* PlayerCtl = Cast<AWorldPlayerController>(Ctl))
			{
				if (UCameraControllerComponent* CamCtrl = PlayerCtl->GetCameraController())
				{
					CamCtrl->SetJumping(false);
				}
			}
		}

		// 停止移动 + 清零速度
		if (AController* Ctl = OwnerChar->GetController())
		{
			Ctl->StopMovement();
		}
		if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		{
			MoveComp->Velocity = FVector::ZeroVector;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Jump ended"));
}

bool UJumpSkill::IsTargetReachable(AActor* Instigator, const FVector& Target) const
{
	if (!Instigator)
	{
		return false;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Instigator->GetWorld());
	if (!NavSys)
	{
		return true;
	}

	FNavLocation NavLocation;
	bool bReachable = NavSys->ProjectPointToNavigation(Target, NavLocation, FVector(200.0f));

	return bReachable;
}