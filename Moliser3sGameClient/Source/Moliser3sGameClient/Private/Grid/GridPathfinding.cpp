#include "Grid/GridPathfinding.h"

namespace
{
	// 4 方向邻接
	const FIntPoint Dir4[4] = {
		FIntPoint(1, 0),
		FIntPoint(-1, 0),
		FIntPoint(0, 1),
		FIntPoint(0, -1)
	};

	// 8 方向邻接（含斜向）
	const FIntPoint Dir8[8] = {
		FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1),
		FIntPoint(1, 1), FIntPoint(1, -1), FIntPoint(-1, 1), FIntPoint(-1, -1)
	};

	// 简单启发式（曼哈顿或切比雪夫）
	float Heuristic(const FIntPoint& A, const FIntPoint& B, bool bDiagonal)
	{
		int32 Dx = FMath::Abs(A.X - B.X);
		int32 Dy = FMath::Abs(A.Y - B.Y);
		if (bDiagonal)
		{
			// 切比雪夫（斜向 1.0 代价）
			return FMath::Max(Dx, Dy) * 1.0f;
		}
		return (float)(Dx + Dy);
	}

	// 手写小顶堆（数组式二叉堆）
	struct FOpenNode
	{
		int32 Index = -1;   // Cells 一维索引
		float FScore = 0.0f;
	};

	class FMinHeap
	{
	public:
		void Push(const FOpenNode& Node)
		{
			Heap.Add(Node);
			SiftUp(Heap.Num() - 1);
		}

		FOpenNode Pop()
		{
			FOpenNode Top = Heap[0];
			Heap[0] = Heap[Heap.Num() - 1];
			Heap.Pop();
			if (Heap.Num() > 0)
			{
				SiftDown(0);
			}
			return Top;
		}

		bool IsEmpty() const { return Heap.Num() == 0; }

	private:
		TArray<FOpenNode> Heap;

		void SiftUp(int32 Idx)
		{
			while (Idx > 0)
			{
				int32 Parent = (Idx - 1) / 2;
				if (Heap[Idx].FScore < Heap[Parent].FScore)
				{
					Swap(Heap[Idx], Heap[Parent]);
					Idx = Parent;
				}
				else
				{
					break;
				}
			}
		}

		void SiftDown(int32 Idx)
		{
			int32 Count = Heap.Num();
			while (true)
			{
				int32 Left = Idx * 2 + 1;
				int32 Right = Idx * 2 + 2;
				int32 Smallest = Idx;
				if (Left < Count && Heap[Left].FScore < Heap[Smallest].FScore) Smallest = Left;
				if (Right < Count && Heap[Right].FScore < Heap[Smallest].FScore) Smallest = Right;
				if (Smallest != Idx)
				{
					Swap(Heap[Idx], Heap[Smallest]);
					Idx = Smallest;
				}
				else
				{
					break;
				}
			}
		}
	};
}

bool GridPathfinding::FindPath(
	const UGridMapData& Grid,
	const FIntPoint& Start,
	const FIntPoint& Goal,
	TArray<FIntPoint>& OutPath,
	bool bAllowDiagonal)
{
	OutPath.Reset();

	const int32 W = Grid.Width;
	const int32 H = Grid.Height;

	if (!Grid.IsValidCoord(Start.X, Start.Y) || !Grid.IsValidCoord(Goal.X, Goal.Y))
	{
		return false;
	}

	// 起点或终点不可走
	if (!Grid.GetCell(Start.X, Start.Y)->IsWalkable() || !Grid.GetCell(Goal.X, Goal.Y)->IsWalkable())
	{
		return false;
	}

	if (Start == Goal)
	{
		OutPath.Add(Start);
		return true;
	}

	const int32 NumCells = Grid.NumCells();

	TArray<float> GScore;
	TArray<float> FScore;
	TArray<int32> CameFrom;          // -1 = 未访问
	TArray<bool> bClosed;
	GScore.Init(TNumericLimits<float>::Max(), NumCells);
	FScore.Init(TNumericLimits<float>::Max(), NumCells);
	CameFrom.Init(-1, NumCells);
	bClosed.Init(false, NumCells);

	const int32 StartIdx = Start.Y * W + Start.X;
	const int32 GoalIdx = Goal.Y * W + Goal.X;

	GScore[StartIdx] = 0.0f;
	FScore[StartIdx] = Heuristic(Start, Goal, bAllowDiagonal);

	FMinHeap Open;
	Open.Push({ StartIdx, FScore[StartIdx] });

	const FIntPoint* Dirs = bAllowDiagonal ? Dir8 : Dir4;
	const int32 DirCount = bAllowDiagonal ? 8 : 4;

	bool bFound = false;

	while (!Open.IsEmpty())
	{
		FOpenNode Current = Open.Pop();
		if (bClosed[Current.Index])
		{
			continue;
		}
		bClosed[Current.Index] = true;

		if (Current.Index == GoalIdx)
		{
			bFound = true;
			break;
		}

		int32 CX = Current.Index % W;
		int32 CY = Current.Index / W;

		for (int32 i = 0; i < DirCount; ++i)
		{
			int32 NX = CX + Dirs[i].X;
			int32 NY = CY + Dirs[i].Y;
			if (!Grid.IsValidCoord(NX, NY))
			{
				continue;
			}

			if (!Grid.IsPassable(CX, CY, NX, NY))
			{
				continue;
			}

			// 斜向移动时，禁止穿过对角墙（拐角穿插）
			if (bAllowDiagonal && Dirs[i].X != 0 && Dirs[i].Y != 0)
			{
				if (!Grid.IsPassable(CX, CY, NX, CY) || !Grid.IsPassable(CX, CY, CX, NY))
				{
					continue;
				}
			}

			int32 NeighborIdx = NY * W + NX;
			if (bClosed[NeighborIdx])
			{
				continue;
			}

			float MoveCost = (Dirs[i].X != 0 && Dirs[i].Y != 0) ? 1.414f : 1.0f;
			const FGridCell* NCell = Grid.GetCell(NX, NY);
			MoveCost *= (NCell ? NCell->Cost : 1.0f);

			float TentativeG = GScore[Current.Index] + MoveCost;
			if (TentativeG < GScore[NeighborIdx])
			{
				GScore[NeighborIdx] = TentativeG;
				FScore[NeighborIdx] = TentativeG + Heuristic(FIntPoint(NX, NY), Goal, bAllowDiagonal);
				CameFrom[NeighborIdx] = Current.Index;
				Open.Push({ NeighborIdx, FScore[NeighborIdx] });
			}
		}
	}

	if (!bFound)
	{
		return false;
	}

	// 回溯路径
	TArray<int32> ReversedIdx;
	int32 Cur = GoalIdx;
	while (Cur != -1)
	{
		ReversedIdx.Add(Cur);
		Cur = CameFrom[Cur];
	}

	for (int32 i = ReversedIdx.Num() - 1; i >= 0; --i)
	{
		int32 Idx = ReversedIdx[i];
		OutPath.Add(FIntPoint(Idx % W, Idx / W));
	}

	return true;
}
