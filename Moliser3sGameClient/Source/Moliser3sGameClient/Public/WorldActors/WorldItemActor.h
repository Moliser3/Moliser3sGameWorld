#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldItemActor.generated.h"

class UItemBase;

UCLASS(Blueprintable)
class MOLISER3SGAMECLIENT_API AWorldItemActor : public AActor
{
	GENERATED_BODY()

public:
	AWorldItemActor();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "物品")
	TObjectPtr<UItemBase> ItemData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "物品")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "物品")
	TObjectPtr<class UWidgetComponent> WidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "物品")
	bool bCanPickup = true;

	UFUNCTION(BlueprintCallable, Category = "物品")
	void InitializeFromItem(UItemBase* InItem);

	UFUNCTION(BlueprintCallable, Category = "物品")
	void OnPickup(AActor* Picker);

	UFUNCTION(BlueprintImplementableEvent, Category = "物品")
	void OnPickupEffect(AActor* Picker);

protected:
	void UpdateVisuals();
};
