#pragma once

#include "CoreMinimal.h"
#include "Grid/GridMapData.h"

/**
 * 网格 A* 寻路（纯逻辑，无 UObject 依赖）
 */
class MOLISER3SGAMECLIENT_API GridPathfinding
{
public:
	/**
	 * 从 Start 到 Goal 计算路径
	 * @param Grid 网格数据
	 * @param Start 起点网格坐标
	 * @param Goal 终点网格坐标
	 * @param bAllowDiagonal 是否允许斜向移动
	 * @param OutPath 输出的路径（含起点，不含终点或含终点，由 bIncludeGoal 决定）
	 * @return 是否找到路径
	 */
	static bool FindPath(
		const UGridMapData& Grid,
		const FIntPoint& Start,
		const FIntPoint& Goal,
		TArray<FIntPoint>& OutPath,
		bool bAllowDiagonal = false);
};
