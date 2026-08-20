#include "Grid/GridMovementComponent.h"
#include "Grid/GridPathfinding.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGridMovementComponent::UGridMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UGridMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bMoving)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		StopGridMovement();
		return;
	}

	// 使用角色移动组件进行平滑移动
	UCharacterMovementComponent* MoveComp = nullptr;
	if (ACharacter* Char = Cast<ACharacter>(Owner))
	{
		MoveComp = Char->GetCharacterMovement();
	}

	FVector OwnerLoc = Owner->GetActorLocation();
	FVector ToWaypoint = CurrentWaypoint - OwnerLoc;
	ToWaypoint.Z = 0.0f;

	float Dist = ToWaypoint.Size();

	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = MoveSpeed;
		FVector Input = ToWaypoint.GetSafeNormal2D();
		Owner->AddActorWorldOffset(Input * MoveSpeed * DeltaTime, true);
	}
	else
	{
		FVector Step = ToWaypoint.GetSafeNormal() * MoveSpeed * DeltaTime;
		Owner->AddActorWorldOffset(Step, true);
	}

	// 到达当前格中心 → 下一个
	if (Dist <= ArriveTolerance)
	{
		ProceedToNextWaypoint();
	}
}

bool UGridMovementComponent::MoveToGridLocation(UGridMapData* Grid, const FVector& WorldDest)
{
	if (!Grid)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	CurrentGrid = Grid;

	FIntPoint Start = Grid->WorldToGrid(Owner->GetActorLocation());
	FIntPoint Goal = Grid->WorldToGrid(WorldDest);

	TArray<FIntPoint> ResultPath;
	if (!GridPathfinding::FindPath(*Grid, Start, Goal, ResultPath, false))
	{
		// 目标不可达，或路径未找到
		Path.Reset();
		PathIndex = -1;
		bMoving = false;
		OnGridMovementFinished.Broadcast();
		return false;
	}

	Path = ResultPath;

	// 若起点==终点（已到达），直接完成
	if (Path.Num() <= 1)
	{
		Path.Reset();
		PathIndex = -1;
		bMoving = false;
		OnGridMovementFinished.Broadcast();
		return true;
	}

	// 跳过当前所在的格子（第0个），从下一个开始走
	PathIndex = 1;
	CurrentWaypoint = Grid->GridToWorld(Path[PathIndex]);
	bMoving = true;

	return true;
}

void UGridMovementComponent::StopGridMovement()
{
	bMoving = false;
	Path.Reset();
	PathIndex = -1;
}

void UGridMovementComponent::ProceedToNextWaypoint()
{
	if (!CurrentGrid)
	{
		StopGridMovement();
		return;
	}

	PathIndex++;
	if (PathIndex >= Path.Num())
	{
		StopGridMovement();
		OnGridMovementFinished.Broadcast();
		return;
	}

	CurrentWaypoint = CurrentGrid->GridToWorld(Path[PathIndex]);
}
