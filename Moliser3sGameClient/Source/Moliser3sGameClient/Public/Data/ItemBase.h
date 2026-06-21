#pragma once

#include "CoreMinimal.h"
#include "ItemBase.generated.h"

/**
 * 所有物品的基类
 * 可在蓝图派生，配置具体物品数据
 */
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew)
class MOLISER3SGAMECLIENT_API UItemBase : public UObject
{
	GENERATED_BODY()

public:
	/** 物品唯一标识 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FName ItemID = NAME_None;

	/** 显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FText DisplayName;

	/** 物品描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FText Description;

	/** 物品图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	TSoftObjectPtr<class UTexture2D> Icon;

	/** 堆叠上限（默认1=不可堆叠） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;
};
