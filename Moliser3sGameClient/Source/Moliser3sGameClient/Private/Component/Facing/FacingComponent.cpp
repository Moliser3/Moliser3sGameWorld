#include "Component/Facing/FacingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "BaseCharacter.h"


UFacingComponent::UFacingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UFacingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFacingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentFacingMode != EFacingMode::Aiming || !AimTargetActor.IsValid())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 距离检测
	float Dist = FVector::Dist(Owner->GetActorLocation(), AimTargetActor->GetActorLocation());

	// 从基类读取解锁距离和停止距离
	float LockRange = 1000.0f;
	float StopDist = 100.0f;
	if (ABaseCharacter* BaseChar = Cast<ABaseCharacter>(Owner))
	{
		LockRange = BaseChar->GetLockOnRange();
		StopDist = BaseChar->GetStopDistance();
	}

	// 超过最大距离 — 自动切回行走模式
	if (Dist > LockRange)
	{
		ClearAimTarget();
		return;
	}

	// 注视朝向 — 平滑转向目标
	FVector Direction = (AimTargetActor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		FRotator TargetRot = Direction.Rotation();
		FRotator NewRot = FMath::RInterpTo(Owner->GetActorRotation(), TargetRot, DeltaTime, RotationSpeed);
		Owner->SetActorRotation(NewRot);
	}
}

void UFacingComponent::SetAimTarget(AActor* Target)
{
	if (!Target)
	{
		ClearAimTarget();
		return;
	}

	AimTargetActor = Target;
	CurrentFacingMode = EFacingMode::Aiming;

	// 关闭旋转跟随移动，让组件控制旋转
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		{
			MoveComp->bOrientRotationToMovement = false;
		}
	}

	// 广播模式改变事件
	OnFacingModeChanged.Broadcast(CurrentFacingMode);
}

void UFacingComponent::ClearAimTarget()
{
	AimTargetActor = nullptr;

	if (CurrentFacingMode != EFacingMode::Walking)
	{
		CurrentFacingMode = EFacingMode::Walking;

		// 恢复旋转跟随移动
		if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
		{
			if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
			{
				MoveComp->bOrientRotationToMovement = true;
			}
		}

		// 广播模式改变事件
		OnFacingModeChanged.Broadcast(CurrentFacingMode);
	}
}

E4Direction UFacingComponent::GetMovementDirection4() const
{
	AActor* Owner = GetOwner();
	if (!Owner || !AimTargetActor.IsValid())
	{
		return E4Direction::Forward;
	}

	// 获取移动速度方向
	FVector VelocityDir = FVector::ZeroVector;
	if (ACharacter* OwnerChar = Cast<ACharacter>(Owner))
	{
		if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		{
			// 只在移动时计算方向
			if (MoveComp->Velocity.IsNearlyZero())
			{
				return E4Direction::Forward;
			}
			VelocityDir = MoveComp->Velocity.GetSafeNormal2D();
		}
	}

	if (VelocityDir.IsNearlyZero())
	{
		return E4Direction::Forward;
	}

	// 计算从角色指向目标的正面方向
	FVector ForwardDir = (AimTargetActor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
	if (ForwardDir.IsNearlyZero())
	{
		return E4Direction::Forward;
	}

	// 计算速度方向相对于正面方向的夹角（度）
	float Dot = FVector::DotProduct(ForwardDir, VelocityDir);
	float Cross = FVector::CrossProduct(ForwardDir, VelocityDir).Z;

	// 将 Dot/Cross 转换为角度（-180° ~ 180°）
	float Angle = FMath::Atan2(Cross, Dot) * (180.0f / UE_PI);

	// 根据角度映射到4方向（45°为一个区间）
	//        前
	//   -45°  |  45°
	//     \   |   /
	//  左  \  |  /  右
	//       \ | /
	//   -135°| 135°
	//        后
	static const float Sector45 = 45.0f;

	if (Angle >= -Sector45 && Angle < Sector45)
	{
		return E4Direction::Forward;
	}
	if (Angle >= Sector45 && Angle < 180.0f - Sector45)
	{
		return E4Direction::Right;
	}
	if (Angle >= 180.0f - Sector45 || Angle < -180.0f + Sector45)
	{
		return E4Direction::Back;
	}
	// Angle >= -180.0f + Sector45 && Angle < -Sector45
	return E4Direction::Left;
}
