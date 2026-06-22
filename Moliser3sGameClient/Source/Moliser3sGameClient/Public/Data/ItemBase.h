#pragma once

#include "CoreMinimal.h"
#include "ItemBase.generated.h"

UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew)
class MOLISER3SGAMECLIENT_API UItemBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FName ItemID = NAME_None;

	UFUNCTION(BlueprintPure, Category = "基础")
	class UTexture2D* GetIcon()
	{
		if (!CachedIcon && !Icon.IsNull())
		{
			CachedIcon = Icon.LoadSynchronous();
		}
		return CachedIcon;
	}

	UFUNCTION(BlueprintPure, Category = "基础")
	class UStaticMesh* GetWorldMesh()
	{
		if (!CachedWorldMesh && !WorldMesh.IsNull())
		{
			CachedWorldMesh = WorldMesh.LoadSynchronous();
		}
		return CachedWorldMesh;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	TSoftObjectPtr<class UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	/** 掉落在地面上的静态网格体 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "世界呈现")
	TSoftObjectPtr<class UStaticMesh> WorldMesh;

	/** 使用物品（BlueprintNativeEvent 让蓝图也可覆盖） */
	UFUNCTION(BlueprintNativeEvent, Category = "物品")
	void Use(AActor* User);
	virtual void Use_Implementation(AActor* User);

protected:
	UPROPERTY(Transient)
	TObjectPtr<class UTexture2D> CachedIcon;

	UPROPERTY(Transient)
	TObjectPtr<class UStaticMesh> CachedWorldMesh;
};
