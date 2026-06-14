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
	SkillCategory = ESkillCategory::Movement;

	// 前摇 = 抛物线飞行（不可打断），后摇 = 落地收尾（可打断）
	WindupTime = 0.72f;
	RecoveryTime = 1.35f;
	CustomLinkTime = 0.2f;
	MaxSkillRange = -1;
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

	if (PlayerCtl)
	{
		PlayerCtl->SetLastClickTarget(FinalTarget);
	}

	bool bReachable = IsTargetReachable(Instigator, FinalTarget);

	if (!bReachable)
	{
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

	// 停止移动
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

	if (Instigator && Instigator->GetWorld())
	{
		UWorld* World = Instigator->GetWorld();
		const int32 NumSamples = 20;
		FVector PrevPoint = JumpStartLoc;
		PrevPoint.Z = JumpStartLoc.Z;
		for (int32 i = 1; i <= NumSamples; i++)
		{
			float T = (float)i / NumSamples;
			FVector HPos = FMath::Lerp(JumpStartLoc, JumpTargetLoc, T);
			HPos.Z = JumpStartLoc.Z;
			float VOffset = 4.0f * JumpHeight * T * (1.0f - T);
			FVector Point = HPos;
			Point.Z += VOffset;

			DrawDebugLine(World, PrevPoint, Point, FColor::Green, false, WindupTime, 0, 1.0f);
			DrawDebugPoint(World, Point, 4.0f, FColor::Green, false, WindupTime);
			PrevPoint = Point;
		}
		DrawDebugSphere(World, JumpStartLoc, 15.0f, 8, FColor::Blue, false, WindupTime);
		DrawDebugSphere(World, JumpTargetLoc, 15.0f, 8, FColor::Red, false, WindupTime);
	}

	// 初始化跳跃状态
	bIsJumping = true;
	JumpProgress = 0.0f;
}

void UJumpSkill::OnWindupUpdate(AActor* Instigator, float DeltaTime)
{
	if (!Instigator || !bIsJumping)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar)
	{
		return;
	}

	// 推进跳跃进度（WindupTime 即总飞行时长）
	if (JumpProgress < 1.0f)
	{
		JumpProgress += DeltaTime / WindupTime;
		if (JumpProgress > 1.0f)
		{
			JumpProgress = 1.0f;
		}

		FVector HorizontalPos = FMath::Lerp(JumpStartLoc, JumpTargetLoc, JumpProgress);
		HorizontalPos.Z = JumpStartLoc.Z;

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
			// 碰撞到障碍物，直接置进度到终点，让 Tick 切换至技能触发
			JumpProgress = 1.0f;
			OwnerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			return;
		}

		OwnerChar->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void UJumpSkill::OnExecute(AActor* Instigator)
{
	if (!Instigator || !bIsJumping)
	{
		return;
	}

	bIsJumping = false;
	JumpProgress = 0.0f;

	ACharacter* OwnerChar = Cast<ACharacter>(Instigator);
	if (!OwnerChar)
	{
		return;
	}

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

	// 停止移动 + 清零速度
	if (AController* Ctl = OwnerChar->GetController())
	{
		Ctl->StopMovement();
	}
	if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
	{
		MoveComp->Velocity = FVector::ZeroVector;
	}

	UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Landed, entering recovery (%.2f s)"), RecoveryTime);
}

void UJumpSkill::OnInterrupt(AActor* Instigator)
{
	if (bIsJumping)
	{
		bIsJumping = false;
		JumpProgress = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("[JumpSkill] Interrupted!"));
	}

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
