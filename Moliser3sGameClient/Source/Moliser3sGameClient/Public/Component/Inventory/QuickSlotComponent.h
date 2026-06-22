#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuickSlotComponent.generated.h"

class UItemBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuickSlotComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "快捷栏", meta = (ClampMin = "1"))
	int32 SlotCount = 8;

	UPROPERTY(BlueprintAssignable, Category = "快捷栏")
	FOnQuickSlotChanged OnQuickSlotChanged;

	UFUNCTION(BlueprintCallable, Category = "快捷栏")
	bool AssignSlot(int32 Index, UItemBase* Item);

	UFUNCTION(BlueprintCallable, Category = "快捷栏")
	void ClearSlot(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "快捷栏")
	void UseSlot(int32 Index);

	UFUNCTION(BlueprintPure, Category = "快捷栏")
	UItemBase* GetSlotItem(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = "快捷栏")
	TArray<UItemBase*> GetAllSlots() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "快捷栏")
	TArray<TObjectPtr<UItemBase>> Slots;
};
