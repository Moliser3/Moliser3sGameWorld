#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/DataDefinitions.h"
#include "Data/EquipmentData.h"
#include "Data/ConsumableItem.h"
#include "ItemDataTable.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "名称"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "描述"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "图标"))
	TSoftObjectPtr<class UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "地面模型"))
	TSoftObjectPtr<class UStaticMesh> WorldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "最大堆叠", ClampMin = "1"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备", meta = (DisplayName = "装备槽位"))
	EEquipmentSlot EquipSlot = EEquipmentSlot::Helmet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备", meta = (DisplayName = "武器类型"))
	EWeaponUsage WeaponUsage = EWeaponUsage::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备", meta = (DisplayName = "等级需求", ClampMin = "1"))
	int32 LevelRequirement = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备", meta = (DisplayName = "稀有度"))
	EItemRarity Rarity = EItemRarity::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五维加成", meta = (DisplayName = "劲力加成"))
	float JinLiBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五维加成", meta = (DisplayName = "气血加成"))
	float QiXueBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五维加成", meta = (DisplayName = "内息加成"))
	float NeiXiBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五维加成", meta = (DisplayName = "身法加成"))
	float ShenFaBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五维加成", meta = (DisplayName = "体魄加成"))
	float TiPoBonus = 0.0f;
};

USTRUCT(BlueprintType)
struct FConsumableDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "名称"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "描述"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "图标"))
	TSoftObjectPtr<class UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "地面模型"))
	TSoftObjectPtr<class UStaticMesh> WorldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "最大堆叠", ClampMin = "1"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "消耗品", meta = (DisplayName = "效果类型"))
	EConsumableEffectType EffectType = EConsumableEffectType::HealHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "消耗品", meta = (DisplayName = "效果值", ClampMin = "0.0"))
	float EffectValue = 50.0f;
};

USTRUCT(BlueprintType)
struct FMaterialDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "名称"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "描述"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "图标"))
	TSoftObjectPtr<class UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "地面模型"))
	TSoftObjectPtr<class UStaticMesh> WorldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "最大堆叠", ClampMin = "1"))
	int32 MaxStackSize = 1;
};

USTRUCT(BlueprintType)
struct FQuestItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "名称"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "描述"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "图标"))
	TSoftObjectPtr<class UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "地面模型"))
	TSoftObjectPtr<class UStaticMesh> WorldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础", meta = (DisplayName = "最大堆叠", ClampMin = "1"))
	int32 MaxStackSize = 1;
};
