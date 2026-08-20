#include "Grid/GridMapData.h"

void UGridMapData::Resize(int32 NewWidth, int32 NewHeight)
{
	if (NewWidth <= 0 || NewHeight <= 0)
	{
		return;
	}

	Width = NewWidth;
	Height = NewHeight;
	Cells.SetNum(Width * Height);
	ResetAll();
}

void UGridMapData::ResetAll()
{
	for (FGridCell& Cell : Cells)
	{
		Cell.Terrain = EGridTerrain::Ground;
		Cell.Height = 0;
		Cell.Cost = 1.0f;
	}
}

FIntPoint UGridMapData::WorldToGrid(const FVector& World) const
{
	// 网格原点位于左下角，X 向右，Y 向上
	int32 X = FMath::FloorToInt((World.X - Origin.X) / CellSize);
	int32 Y = FMath::FloorToInt((World.Y - Origin.Y) / CellSize);
	return FIntPoint(X, Y);
}

FVector UGridMapData::GridToWorld(const FIntPoint& Grid) const
{
	// 返回格中心的世界坐标
	float CenterX = Origin.X + (Grid.X + 0.5f) * CellSize;
	float CenterY = Origin.Y + (Grid.Y + 0.5f) * CellSize;
	return FVector(CenterX, CenterY, Origin.Z);
}

bool UGridMapData::IsPassable(int32 FromX, int32 FromY, int32 ToX, int32 ToY) const
{
	const FGridCell* From = GetCell(FromX, FromY);
	const FGridCell* To = GetCell(ToX, ToY);
	if (!From || !To)
	{
		return false;
	}

	if (!From->IsWalkable() || !To->IsWalkable())
	{
		return false;
	}

	// 高度差规则：
	//   差 0        → 平地可走
	//   差 1        → 需是斜坡（两格任一为 Slope）
	//   差 > 1      → 悬崖不可走
	int32 HeightDiff = FMath::Abs((int32)From->Height - (int32)To->Height);
	if (HeightDiff == 0)
	{
		return true;
	}
	if (HeightDiff == 1)
	{
		return From->Terrain == EGridTerrain::Slope || To->Terrain == EGridTerrain::Slope;
	}
	return false;
}
