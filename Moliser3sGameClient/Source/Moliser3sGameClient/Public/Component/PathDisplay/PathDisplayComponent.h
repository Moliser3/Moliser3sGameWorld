#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PathDisplayComponent.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UPathDisplayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPathDisplayComponent();

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void SetNewPathMesh(const TArray<FVector>& InPathNodes, float InStepLength, float InPathWidth, float InEndMarkerSize);

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void GeneratePath(const TArray<FVector>& InPathNodes);

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void ClearPath();

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void SetPathWidth(float NewWidth);

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void SetStepLength(float NewLength);

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void SetStepSubdivision(int32 NewSubdivision);

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void SetPathMaterial(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void SetEndMarkerMaterial(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintCallable, Category = "PathDisplay")
	void SetEndMarkerSize(float NewSize);

	UFUNCTION(BlueprintPure, Category = "PathDisplay")
	const TArray<FVector>& GetPathNodes() const { return PathNodes; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PathDisplay")
	float PathWidth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PathDisplay", meta = (ClampMin = "0"))
	float StepLength = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PathDisplay", meta = (ClampMin = "0"))
	int32 StepSubdivision = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PathDisplay", meta = (ClampMin = "0"))
	float EndMarkerSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PathDisplay")
	TObjectPtr<UMaterialInterface> PathMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PathDisplay")
	TObjectPtr<UMaterialInterface> EndMarkerMaterial = nullptr;

	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> MeshComponent = nullptr;

	TArray<FVector> PathNodes;

	void BuildPathMesh();
	void BuildEndMarkerSection();

	FVector CatmullRomPoint(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T);
	FVector CatmullRomTangent(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T);

	void AddQuad(
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UVs,
		const FVector& InBL,
		const FVector& InBR,
		const FVector& InTL,
		const FVector& InTR,
		float V0,
		float V1,
		bool bAlternateWinding
	);
};
