#include "WorldActors/WorldItemActor.h"
#include "Data/ItemBase.h"
#include "Component/Inventory/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"

AWorldItemActor::AWorldItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	SetRootComponent(MeshComponent);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(MeshComponent);
	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawSize(FVector2D(40.0f, 40.0f));
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWorldItemActor::BeginPlay()
{
	Super::BeginPlay();
	UpdateVisuals();
}

void AWorldItemActor::InitializeFromItem(UItemBase* InItem)
{
	ItemData = InItem;
	UpdateVisuals();
}

void AWorldItemActor::OnPickup(AActor* Picker)
{
	if (!bCanPickup || !Picker || !ItemData) return;

	UInventoryComponent* InvComp = Picker->FindComponentByClass<UInventoryComponent>();
	if (!InvComp) return;

	if (InvComp->AddItem(ItemData))
	{
		OnPickupEffect(Picker);
		Destroy();
	}
}

void AWorldItemActor::UpdateVisuals()
{
	if (!ItemData) return;

	// 设置静态网格体
	UStaticMesh* Mesh = ItemData->GetWorldMesh();
	if (Mesh)
	{
		MeshComponent->SetStaticMesh(Mesh);
	}
}
