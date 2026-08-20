#include "WorldActors/GridTestActor.h"
#include "Grid/GridPathfinding.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"

AGridTestActor::AGridTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 自身只作网格原点/控制器，不生成移动物体
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void AGridTestActor::BeginPlay()
{
	Super::BeginPlay();

	BuildTestGrid();
	RunTestPath();
}

void AGridTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bMoving || !GridData || !TargetActor)
	{
		return;
	}

	// 到达当前格后等待
	if (WaitTimer > 0.0f)
	{
		WaitTimer -= DeltaTime;
		return;
	}

	FVector ActorLoc = TargetActor->GetActorLocation();
	FVector ToTarget = TargetWaypoint - ActorLoc;
	ToTarget.Z = 0.0f;
	float Dist = ToTarget.Size();

	if (Dist <= 1.0f)
	{
		// 到达当前格中心
		TargetActor->SetActorLocation(FVector(TargetWaypoint.X, TargetWaypoint.Y, ActorLoc.Z));
		WaitTimer = WaitTimeAtCell;

		if (!AdvanceToNextWaypoint())
		{
			// 到达终点
			if (bLoopMovement)
			{
				bForward = !bForward;
				StartPathMove(Path.Last(), Path[0]);
			}
			else
			{
				bMoving = false;
				UE_LOG(LogTemp, Warning, TEXT("[GridTest] 到达终点，停止移动"));
			}
		}
		return;
	}

	FVector Step = ToTarget.GetSafeNormal() * MoveSpeed * DeltaTime;
	TargetActor->SetActorLocation(ActorLoc + Step);
}

void AGridTestActor::BuildTestGrid()
{
	GridData = NewObject<UGridMapData>(this);
	GridData->Width = GridSizeX;
	GridData->Height = GridSizeY;
	GridData->CellSize = CellSize;
	GridData->Origin = GetActorLocation();
	GridData->Cells.SetNum(GridData->NumCells());
	GridData->ResetAll();

	auto CellAt = [&](int32 X, int32 Y) -> FGridCell*
	{
		return GridData->GetCell(X, Y);
	};

	// 一堵竖直墙（中间，X=10，从 Y=3 到 Y=16，留缺口）
	for (int32 Y = 3; Y <= 16; ++Y)
	{
		if (Y == 9 || Y == 10) continue; // 缺口
		FGridCell* C = CellAt(10, Y);
		if (C) C->Terrain = EGridTerrain::Wall;
	}

	// 一块悬崖高地（左上角区域，高度=2）
	for (int32 X = 2; X <= 6; ++X)
		for (int32 Y = 14; Y <= 17; ++Y)
		{
			FGridCell* C = CellAt(X, Y);
			if (C) { C->Height = 2; C->Terrain = EGridTerrain::Ground; }
		}

	// 连接悬崖的斜坡（高地南侧一行，高度=1，设为斜坡）
	for (int32 X = 2; X <= 6; ++X)
	{
		FGridCell* C = CellAt(X, 13);
		if (C) { C->Height = 1; C->Terrain = EGridTerrain::Slope; }
	}

	// 一块水面（右下角不可走）
	for (int32 X = 15; X <= 18; ++X)
		for (int32 Y = 15; Y <= 18; ++Y)
		{
			FGridCell* C = CellAt(X, Y);
			if (C) C->Terrain = EGridTerrain::Water;
		}

	if (bDrawDebug)
	{
		DrawGridVisuals();
	}
}

void AGridTestActor::RunTestPath()
{
	if (!GridData)
	{
		return;
	}

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GridTest] 未指定 TargetActor，无法移动"));
		return;
	}

	if (!GridData->IsValidCoord(TestStart.X, TestStart.Y) || !GridData->IsValidCoord(TestGoal.X, TestGoal.Y))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GridTest] 起终测试点越界"));
		return;
	}

	TArray<FIntPoint> Result;
	bool bFound = GridPathfinding::FindPath(*GridData, TestStart, TestGoal, Result, false);

	UE_LOG(LogTemp, Warning, TEXT("[GridTest] FindPath %s -> %s : %s"),
		*TestStart.ToString(), *TestGoal.ToString(), bFound ? TEXT("成功") : TEXT("失败"));

	if (!bFound)
	{
		return;
	}

	// 绘制路径线
	if (bDrawDebug)
	{
		for (int32 i = 0; i + 1 < Result.Num(); ++i)
		{
			FVector A = GetCellWorldCenter(Result[i]);
			FVector B = GetCellWorldCenter(Result[i + 1]);
			A.Z += 50.0f;
			B.Z += 50.0f;
			DrawDebugLine(GetWorld(), A, B, FColor::Magenta, true, -1.0f, 0, 3.0f);
		}
	}

	Path = Result;
	bForward = true;
	StartPathMove(Path[0], Path[1]);
}

bool AGridTestActor::AdvanceToNextWaypoint()
{
	if (bForward)
	{
		PathIndex++;
		if (PathIndex >= Path.Num())
		{
			return false; // 到达终点
		}
		TargetWaypoint = GetCellWorldCenter(Path[PathIndex]);
		return true;
	}
	else
	{
		PathIndex--;
		if (PathIndex < 0)
		{
			return false; // 回到起点
		}
		TargetWaypoint = GetCellWorldCenter(Path[PathIndex]);
		return true;
	}
}

void AGridTestActor::StartPathMove(const FIntPoint& From, const FIntPoint& To)
{
	if (!TargetActor)
	{
		return;
	}

	// 把物体放到起点格中心
	FVector StartPos = GetCellWorldCenter(From);
	FVector CurrentLoc = TargetActor->GetActorLocation();
	TargetActor->SetActorLocation(FVector(StartPos.X, StartPos.Y, CurrentLoc.Z));

	if (bForward)
	{
		PathIndex = 0;
	}
	else
	{
		PathIndex = Path.Num() - 1;
	}

	bMoving = true;
	WaitTimer = 0.0f;
	AdvanceToNextWaypoint();
}

FVector AGridTestActor::GetCellWorldCenter(const FIntPoint& Cell) const
{
	return GridData ? GridData->GridToWorld(Cell) : GetActorLocation();
}

void AGridTestActor::DrawGridVisuals()
{
	if (!GridData)
	{
		return;
	}

	FColor Color;
	for (int32 Y = 0; Y < GridData->Height; ++Y)
	{
		for (int32 X = 0; X < GridData->Width; ++X)
		{
			const FGridCell* Cell = GridData->GetCell(X, Y);
			if (!Cell) continue;

			switch (Cell->Terrain)
			{
			case EGridTerrain::Wall:  Color = FColor::Red;    break;
			case EGridTerrain::Water: Color = FColor::Blue;   break;
			case EGridTerrain::Slope: Color = FColor::Yellow; break;
			default:                  Color = FColor::Green;  break;
			}

			FVector Center = GetCellWorldCenter(FIntPoint(X, Y));
			Center.Z += (Cell->Height * 25.0f);
			float Half = GridData->CellSize * 0.5f - 1.0f;
			FVector Extent(Half, Half, 1.0f);
			DrawDebugBox(GetWorld(), Center, Extent, Color, true, -1.0f, 0, 1.0f);
		}
	}
}
