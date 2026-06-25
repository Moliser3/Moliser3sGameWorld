#pragma once

#include "CoreMinimal.h"
#include "Data/ItemBase.h"
#include "ConsumableItem.generated.h"

/** 消耗品效果类型 */
UENUM(BlueprintType)
enum class EConsumableEffectType : uint8
{
	HealHP   UMETA(DisplayName = "恢复血量"),
	RestoreMP UMETA(DisplayName = "恢复法力"),
	Buff     UMETA(DisplayName = "增益效果")
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOLISER3SGAMECLIENT_API UConsumableItem : public UItemBase
{
	GENERATED_BODY()

public:
	UConsumableItem();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "消耗品", meta = (DisplayName = "效果类型"))
	EConsumableEffectType EffectType = EConsumableEffectType::HealHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "消耗品", meta = (DisplayName = "效果值", ClampMin = "0.0"))
	float EffectValue = 50.0f;

	virtual void Use_Implementation(AActor* User) override;
};
