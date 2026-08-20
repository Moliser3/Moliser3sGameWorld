#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GridMapData.generated.h"

/** 网格地形类型 */
UENUM(BlueprintType)
enum class EGridTerrain : uint8
{
	Ground  UMETA(DisplayName = "平地"),
	Slope   UMETA(DisplayName = "斜坡"),
	Wall    UMETA(DisplayName = "墙/不可走"),
	Water   UMETA(DisplayName = "水面")
};

/** 单个网格单元 */
USTRUCT(BlueprintType)
struct FGridCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
	EGridTerrain Terrain = EGridTerrain::Ground;

	/** 高度值，相邻格子高度差 > 1 视为悬崖不可走 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
	int32 Height = 0;

	/** 移动代价（A* 权重用，越大越难走） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
	float Cost = 1.0f;

	bool IsWalkable() const
	{
		return Terrain != EGridTerrain::Wall && Terrain != EGridTerrain::Water;
	}
};

/**
 * 网格地图数据
 * 每张地图一份，CellSize 可编辑（默认 25cm = 0.25m）
 */
UCLASS(BlueprintType)
class MOLISER3SGAMECLIENT_API UGridMapData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 网格列数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 Width = 64;

	/** 网格行数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 Height = 64;

	/** 每格世界尺寸（cm），默认 25 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "1.0"))
	float CellSize = 25.0f;

	/** 网格世界原点（通常为地图左下角） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FVector Origin = FVector::ZeroVector;

	/** 所有网格单元（Width * Height） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FGridCell> Cells;

	// ===== 工具 =====
	int32 NumCells() const { return Width * Height; }
	void Resize(int32 NewWidth, int32 NewHeight);
	void ResetAll();

	bool IsValidCoord(int32 X, int32 Y) const
	{
		return X >= 0 && X < Width && Y >= 0 && Y < Height;
	}

	FGridCell* GetCell(int32 X, int32 Y)
	{
		return IsValidCoord(X, Y) ? &Cells[Y * Width + X] : nullptr;
	}

	const FGridCell* GetCell(int32 X, int32 Y) const
	{
		return IsValidCoord(X, Y) ? &Cells[Y * Width + X] : nullptr;
	}

	/** 世界坐标 → 网格坐标（取整，可越界） */
	FIntPoint WorldToGrid(const FVector& World) const;

	/** 网格坐标 → 世界坐标（格中心） */
	FVector GridToWorld(const FIntPoint& Grid) const;

	/** 从网格到目标方向，判断相邻两格是否可通行（含高度/斜坡/悬崖规则） */
	bool IsPassable(int32 FromX, int32 FromY, int32 ToX, int32 ToY) const;
};
