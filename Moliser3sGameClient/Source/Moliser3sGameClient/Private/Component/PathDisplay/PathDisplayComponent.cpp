#include "Component/PathDisplay/PathDisplayComponent.h"
#include "ProceduralMeshComponent.h"
#include "Engine/World.h"

UPathDisplayComponent::UPathDisplayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPathDisplayComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPathDisplayComponent::SetNewPathMesh(const TArray<FVector>& InPathNodes, float InStepLength, float InPathWidth, float InEndMarkerSize)
{
	if (MeshComponent)
	{
		MeshComponent->ClearMeshSection(0);
		MeshComponent->ClearMeshSection(1);
	}

	StepLength = InStepLength;
	PathWidth = InPathWidth;
	EndMarkerSize = InEndMarkerSize;
	GeneratePath(InPathNodes);
}

void UPathDisplayComponent::GeneratePath(const TArray<FVector>& InPathNodes)
{
	PathNodes = InPathNodes;

	if (!MeshComponent)
	{
		MeshComponent = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("PathMesh"));
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetAbsolute(true, true, true);
		MeshComponent->RegisterComponent();
	}

	BuildPathMesh();
	BuildEndMarkerSection();
}

void UPathDisplayComponent::ClearPath()
{
	if (MeshComponent)
	{
		MeshComponent->ClearMeshSection(0);
		MeshComponent->ClearMeshSection(1);
		MeshComponent->SetVisibility(false);
	}
	PathNodes.Empty();
}

void UPathDisplayComponent::SetPathWidth(float NewWidth)
{
	PathWidth = NewWidth;
	if (PathNodes.Num() > 0)
	{
		BuildPathMesh();
	}
}

void UPathDisplayComponent::SetStepLength(float NewLength)
{
	StepLength = NewLength;
	if (PathNodes.Num() > 0)
	{
		BuildPathMesh();
	}
}

void UPathDisplayComponent::SetStepSubdivision(int32 NewSubdivision)
{
	StepSubdivision = FMath::Max(0, NewSubdivision);
	if (PathNodes.Num() > 0)
	{
		BuildPathMesh();
	}
}

void UPathDisplayComponent::SetEndMarkerSize(float NewSize)
{
	EndMarkerSize = FMath::Max(0.0f, NewSize);
	if (PathNodes.Num() > 1 && MeshComponent)
	{
		BuildEndMarkerSection();
	}
}

void UPathDisplayComponent::SetEndMarkerMaterial(UMaterialInterface* NewMaterial)
{
	EndMarkerMaterial = NewMaterial;
	if (MeshComponent && EndMarkerMaterial)
	{
		MeshComponent->SetMaterial(1, EndMarkerMaterial);
	}
}

void UPathDisplayComponent::SetPathMaterial(UMaterialInterface* NewMaterial)
{
	PathMaterial = NewMaterial;
	if (MeshComponent && PathMaterial)
	{
		MeshComponent->SetMaterial(0, PathMaterial);
	}
}

FVector UPathDisplayComponent::CatmullRomPoint(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T)
{
	float T2 = T * T;
	float T3 = T2 * T;
	return 0.5f * (
		(2.0f * P1) +
		(-P0 + P2) * T +
		(2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * T2 +
		(-P0 + 3.0f * P1 - 3.0f * P2 + P3) * T3
	);
}

FVector UPathDisplayComponent::CatmullRomTangent(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T)
{
	float T2 = T * T;
	return 0.5f * (
		(-P0 + P2) +
		2.0f * (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * T +
		3.0f * (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * T2
	);
}

void UPathDisplayComponent::BuildPathMesh()
{
	if (!MeshComponent || PathNodes.Num() < 2) return;

	MeshComponent->ClearMeshSection(0);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;

	const int32 NumSegments = PathNodes.Num() - 1;
	const int32 SubSegCount = FMath::Max(1, StepSubdivision + 1);

	float CumulativeDist = 0.0f;
	int32 QuadCounter = 0;

	for (int32 SegIdx = 0; SegIdx < NumSegments; ++SegIdx)
	{
		int32 P0 = FMath::Max(0, SegIdx - 1);
		int32 P1 = SegIdx;
		int32 P2 = SegIdx + 1;
		int32 P3 = FMath::Min(PathNodes.Num() - 1, SegIdx + 2);

		float SegLen = FVector::Dist(PathNodes[P1], PathNodes[P2]);
		int32 NumSteps = FMath::Max(1, FMath::CeilToInt(SegLen / StepLength));

		for (int32 j = 0; j < NumSteps; ++j)
		{
			float OrigT0 = (float)j / (float)NumSteps;
			float OrigT1 = (float)(j + 1) / (float)NumSteps;

			for (int32 s = 0; s < SubSegCount; ++s)
			{
				float FineT0 = OrigT0 + (OrigT1 - OrigT0) * (float)s / (float)SubSegCount;
				float FineT1 = OrigT0 + (OrigT1 - OrigT0) * (float)(s + 1) / (float)SubSegCount;

				FVector Pt0 = CatmullRomPoint(PathNodes[P0], PathNodes[P1], PathNodes[P2], PathNodes[P3], FineT0);
				FVector Tan0 = CatmullRomTangent(PathNodes[P0], PathNodes[P1], PathNodes[P2], PathNodes[P3], FineT0);
				FVector Pt1 = CatmullRomPoint(PathNodes[P0], PathNodes[P1], PathNodes[P2], PathNodes[P3], FineT1);
				FVector Tan1 = CatmullRomTangent(PathNodes[P0], PathNodes[P1], PathNodes[P2], PathNodes[P3], FineT1);

				FVector Dir0 = Tan0.GetSafeNormal2D();
				if (Dir0.IsNearlyZero())
				{
					Dir0 = (Pt1 - Pt0).GetSafeNormal2D();
				}
				if (Dir0.IsNearlyZero()) Dir0 = FVector::ForwardVector;

				FVector Dir1 = Tan1.GetSafeNormal2D();
				if (Dir1.IsNearlyZero()) Dir1 = Dir0;

				FVector Perp0(-Dir0.Y, Dir0.X, 0.0f);
				FVector Perp1(-Dir1.Y, Dir1.X, 0.0f);

				FVector BL = Pt0 - Perp0 * PathWidth * 0.5f;
				FVector BR = Pt0 + Perp0 * PathWidth * 0.5f;
				FVector TL = Pt1 - Perp1 * PathWidth * 0.5f;
				FVector TR = Pt1 + Perp1 * PathWidth * 0.5f;

				float V0 = CumulativeDist / StepLength;
				float SegDist = FVector::Dist(Pt1, Pt0);
				CumulativeDist += SegDist;
				float V1 = CumulativeDist / StepLength;

				bool bAlternate = (QuadCounter % 2 == 1);
				AddQuad(Vertices, Triangles, Normals, UVs, BL, BR, TL, TR, V0, V1, bAlternate);

				FVector TangentX = (BR - BL).GetSafeNormal();
				if (TangentX.IsNearlyZero()) TangentX = FVector(1.0f, 0.0f, 0.0f);
				FProcMeshTangent Tangent(TangentX, false);
				Tangents.Add(Tangent);
				Tangents.Add(Tangent);
				Tangents.Add(Tangent);
				Tangents.Add(Tangent);

				QuadCounter++;
			}
		}
	}

	if (Vertices.Num() == 0) return;

	MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), Tangents, true);

	if (PathMaterial)
	{
		MeshComponent->SetMaterial(0, PathMaterial);
	}

	if (!MeshComponent->IsVisible())
	{
		MeshComponent->SetVisibility(true);
	}
}

void UPathDisplayComponent::BuildEndMarkerSection()
{
	if (!MeshComponent || PathNodes.Num() < 2) return;

	MeshComponent->ClearMeshSection(1);

	FVector Center = PathNodes.Last();
	float HalfSize = EndMarkerSize * 0.5f;

	TArray<FVector> Verts;
	TArray<int32> Tris;
	TArray<FVector> Norms;
	TArray<FVector2D> UVArr;
	TArray<FProcMeshTangent> Tangents;

	Verts.Add(Center + FVector(-HalfSize, -HalfSize, 0.0f));
	Verts.Add(Center + FVector(HalfSize, -HalfSize, 0.0f));
	Verts.Add(Center + FVector(-HalfSize, HalfSize, 0.0f));
	Verts.Add(Center + FVector(HalfSize, HalfSize, 0.0f));

	Tris.Add(0); Tris.Add(3); Tris.Add(1);
	Tris.Add(0); Tris.Add(2); Tris.Add(3);

	for (int32 i = 0; i < 4; ++i)
	{
		Norms.Add(FVector(0.0f, 0.0f, 1.0f));
	}

	UVArr.Add(FVector2D(0.0f, 0.0f));
	UVArr.Add(FVector2D(1.0f, 0.0f));
	UVArr.Add(FVector2D(0.0f, 1.0f));
	UVArr.Add(FVector2D(1.0f, 1.0f));

	FProcMeshTangent Tangent(FVector(1.0f, 0.0f, 0.0f), false);
	for (int32 i = 0; i < 4; ++i) Tangents.Add(Tangent);

	MeshComponent->CreateMeshSection(1, Verts, Tris, Norms, UVArr, TArray<FColor>(), Tangents, false);

	if (EndMarkerMaterial)
	{
		MeshComponent->SetMaterial(1, EndMarkerMaterial);
	}
	else if (PathMaterial)
	{
		MeshComponent->SetMaterial(1, PathMaterial);
	}
}

void UPathDisplayComponent::AddQuad(
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
	bool bAlternateWinding)
{
	int32 Idx = Vertices.Num();

	Vertices.Add(InBL);
	Vertices.Add(InBR);
	Vertices.Add(InTL);
	Vertices.Add(InTR);

	if (bAlternateWinding)
	{
		Triangles.Add(Idx);
		Triangles.Add(Idx + 1);
		Triangles.Add(Idx + 3);

		Triangles.Add(Idx + 2);
		Triangles.Add(Idx);
		Triangles.Add(Idx + 3);
	}
	else
	{
		Triangles.Add(Idx);
		Triangles.Add(Idx + 1);
		Triangles.Add(Idx + 2);

		Triangles.Add(Idx + 3);
		Triangles.Add(Idx + 2);
		Triangles.Add(Idx + 1);
	}

	FVector Normal(0.0f, 0.0f, 1.0f);
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);

	UVs.Add(FVector2D(0.0f, V0));
	UVs.Add(FVector2D(1.0f, V0));
	UVs.Add(FVector2D(0.0f, V1));
	UVs.Add(FVector2D(1.0f, V1));
}
