// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Camera/CameraControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "PlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UCameraControllerComponent::UCameraControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UCameraControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 初始化相机位置为玩家当前位置 + 固定偏移
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC)
	{
		if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn()))
		{
			if (UCameraComponent* Camera = PlayerChar->GetCamera())
			{
				// 将相机从角色组件层级分离，使其完全独立于角色坐标系
				Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				// 计算初始相机位置
				float PitchRad = FMath::DegreesToRadians(PitchAngle);
				float HorizontalDist = ArmLength * FMath::Cos(PitchRad);
				float VerticalDist = ArmLength * FMath::Sin(PitchRad);
				FVector CameraOffset = FVector(-HorizontalDist, 0.0f, VerticalDist);

				FVector CharPos = PlayerChar->GetActorLocation();
				FVector InitCamPos = CharPos + CameraOffset;

				Camera->SetWorldLocation(InitCamPos);
				Camera->SetWorldRotation(FRotator(-PitchAngle, 0.0f, 0.0f));

				// 关闭运动模糊，防止角色快速移动/跳跃时产生模糊拖影
				Camera->PostProcessSettings.bOverride_MotionBlurAmount = true;
				Camera->PostProcessSettings.MotionBlurAmount = 0.0f;

				CachedCamera = Camera;
			}
		}
	}
}

void UCameraControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return;
	}

	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
	if (!PlayerChar)
	{
		return;
	}

	// 缓存相机引用
	if (!CachedCamera.IsValid())
	{
		CachedCamera = PlayerChar->GetCamera();
		if (!CachedCamera.IsValid())
		{
			return;
		}
	}

	UCameraComponent* Camera = CachedCamera.Get();
	FVector CharPos = PlayerChar->GetActorLocation();
	FVector CurrentCamPos = Camera->GetComponentLocation();

	// ── 计算理想相机位置 ──
	// 相机以固定角度（PitchAngle）和距离（ArmLength）位于角色后上方
	float PitchRad = FMath::DegreesToRadians(PitchAngle);
	float HorizontalDist = ArmLength * FMath::Cos(PitchRad);
	float VerticalDist = ArmLength * FMath::Sin(PitchRad);
	FVector CameraOffset = FVector(-HorizontalDist, 0.0f, VerticalDist);
	FVector TargetCamPos = CharPos + CameraOffset;

	// ── 误差向量 ──
	FVector Error = TargetCamPos - CurrentCamPos;

	// ── 水平弹性（X/Y）──
	FVector ErrorXY = Error;
	ErrorXY.Z = 0;

	FVector TargetVelXY = ErrorXY * HorizontalStiffness;
	float TargetSpeedXY = TargetVelXY.Size();
	if (TargetSpeedXY > HorizontalMaxSpeed)
	{
		TargetVelXY = TargetVelXY.GetSafeNormal() * HorizontalMaxSpeed;
	}

	float ResponseFactorX = FMath::Clamp(HorizontalResponseRate * DeltaTime, 0.0f, 1.0f);
	FVector CurrentVelXY = CameraVelocity;
	CurrentVelXY.Z = 0;
	FVector NewVelXY = FMath::Lerp(CurrentVelXY, TargetVelXY, ResponseFactorX);

	// ── 垂直弹性（Z）──
	float ErrorZ = Error.Z;
	float TargetVelZ = 0.0f;

	if (bIsJumping && bBypassZElasticOnJump)
	{
		// 跳跃中：Z 轴直接跟随，不经过弹性
		TargetVelZ = ErrorZ; // 瞬间到位
	}
	else
	{
		TargetVelZ = ErrorZ * VerticalStiffness;
	}

	// 限制垂直速度
	TargetVelZ = FMath::Clamp(TargetVelZ, -VerticalMaxSpeed, VerticalMaxSpeed);

	float ResponseFactorZ = FMath::Clamp(VerticalResponseRate * DeltaTime, 0.0f, 1.0f);
	float NewVelZ = FMath::Lerp(CameraVelocity.Z, TargetVelZ, ResponseFactorZ);

	// ── 合成速度 ──
	CameraVelocity = FVector(NewVelXY.X, NewVelXY.Y, NewVelZ);

	// ── 根据速度移动相机 ──
	FVector NewCamPos = CurrentCamPos + CameraVelocity * DeltaTime;
	Camera->SetWorldLocation(NewCamPos);

	// ── 固定相机旋转（轴不变）──
	Camera->SetWorldRotation(FRotator(-PitchAngle, 0.0f, 0.0f));
}