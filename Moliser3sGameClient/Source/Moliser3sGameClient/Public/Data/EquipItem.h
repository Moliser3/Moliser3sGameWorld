#pragma once

#include "CoreMinimal.h"
#include "Data/ItemBase.h"
#include "Data/EquipmentData.h"
#include "EquipItem.generated.h"

/**
 * 可装备物品
 * 提供五行加成，蓝图配置不同部位/武器类型的装备
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOLISER3SGAMECLIENT_API UEquipItem : public UItemBase
{
	GENERATED_BODY()

public:
	/** 装备槽位 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备")
	EEquipmentSlot Slot = EEquipmentSlot::Helmet;

	/** 武器类型（仅主手武器有效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备")
	EWeaponUsage WeaponUsage = EWeaponUsage::OneHand;

	/** 等级需求 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备", meta = (ClampMin = "1"))
	int32 LevelRequirement = 1;

	// ===== 五行加成 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五行加成", meta = (DisplayName = "金加成"))
	int32 JinBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五行加成", meta = (DisplayName = "木加成"))
	int32 MuBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五行加成", meta = (DisplayName = "水加成"))
	int32 ShuiBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五行加成", meta = (DisplayName = "火加成"))
	int32 HuoBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备|五行加成", meta = (DisplayName = "土加成"))
	int32 TuBonus = 0;

	/** 是否为双手武器 */
	bool bIsTwoHanded() const { return Slot == EEquipmentSlot::MainHand && WeaponUsage == EWeaponUsage::TwoHand; }

	/** 是否为副手物品 */
	bool bIsOffHand() const { return Slot == EEquipmentSlot::OffHand; }
};
