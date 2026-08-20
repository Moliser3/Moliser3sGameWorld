#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Grid/GridMapData.h"
#include "GridMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGridMovementFinished);

/**
 * 网格路径移动组件
 * 挂在 ACharacter 上，沿 GridPathfinding 算出的路径逐格移动
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UGridMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGridMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 计算并开始沿网格路径移动
	 * @param Grid 网格数据
	 * @param WorldDest 世界坐标目标点
	 * @return 是否成功计算路径
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool MoveToGridLocation(UGridMapData* Grid, const FVector& WorldDest);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void StopGridMovement();

	/** 是否正在移动 */
	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsMoving() const { return bMoving; }

	/** 移动速度（cm/s） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float MoveSpeed = 300.0f;

	/** 到达格子中心的容差（cm） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float ArriveTolerance = 5.0f;

	UPROPERTY(BlueprintAssignable, Category = "Grid")
	FOnGridMovementFinished OnGridMovementFinished;

protected:
	void StartMovingAlongPath();
	void ProceedToNextWaypoint();

	UPROPERTY()
	TObjectPtr<UGridMapData> CurrentGrid;

	TArray<FIntPoint> Path;
	int32 PathIndex = -1;
	FVector CurrentWaypoint = FVector::ZeroVector;
	bool bMoving = false;
};
