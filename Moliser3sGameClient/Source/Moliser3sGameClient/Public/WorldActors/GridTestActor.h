#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/GridMapData.h"
#include "GridTestActor.generated.h"

class UStaticMeshComponent;
class AActor;

/**
 * 网格测试 Actor
 * 程序化生成一个简单网格并可视化（可走=绿、不可走=红、斜坡=黄）
 * 控制一个外部 StaticMesh 物体（TargetMesh）沿 A* 路径从起点逐格移动到终点，循环往复
 */
UCLASS()
class MOLISER3SGAMECLIENT_API AGridTestActor : public AActor
{
	GENERATED_BODY()

public:
	AGridTestActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 生成网格数据（测试用） */
	UFUNCTION(BlueprintCallable, Category = "GridTest")
	void BuildTestGrid();

	/** 计算起点到终点的路径并开始移动 */
	UFUNCTION(BlueprintCallable, Category = "GridTest")
	void RunTestPath();

	/** 是否显示调试盒可视化 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	int32 GridSizeX = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	int32 GridSizeY = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	float CellSize = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	FIntPoint TestStart = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	FIntPoint TestGoal = FIntPoint(18, 18);

	/** 移动物体的速度（cm/s） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	float MoveSpeed = 200.0f;

	/** 到达格中心后等待的时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	float WaitTimeAtCell = 0.1f;

	/** 到达终点后是否循环（反向走回起点） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	bool bLoopMovement = true;

	/** 要控制的移动物体（在场景中指定，如一个 Sphere Static Mesh Actor） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridTest")
	TObjectPtr<AActor> TargetActor;

	/** 生成的网格数据 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridTest")
	TObjectPtr<UGridMapData> GridData;

protected:
	void DrawGridVisuals();
	void StartPathMove(const FIntPoint& From, const FIntPoint& To);
	bool AdvanceToNextWaypoint();
	FVector GetCellWorldCenter(const FIntPoint& Cell) const;

	/** 当前移动的路径（网格坐标序列） */
	TArray<FIntPoint> Path;
	int32 PathIndex = 0;
	bool bMoving = false;
	bool bForward = true;
	FVector TargetWaypoint = FVector::ZeroVector;
	float WaitTimer = 0.0f;
};
