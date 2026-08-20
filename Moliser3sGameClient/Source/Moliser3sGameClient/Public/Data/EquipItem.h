#pragma once

#include "CoreMinimal.h"
#include "Data/ItemBase.h"
#include "Data/EquipmentData.h"
#include "EquipItem.generated.h"

/**
 * 可装备物品
 * 提供五维加成，蓝图配置不同部位/武器类型的装备
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOLISER3SGAMECLIENT_API UEquipItem : public UItemBase
{
	GENERATED_BODY()

public:
	/** 装备槽位 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备", meta = (DisplayName = "装备槽位"))
	EEquipmentSlot Slot = EEquipmentSlot::Helmet;

	/** 武器类型（仅主手武器有效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备", meta = (DisplayName = "武器类型"))
	EWeaponUsage WeaponUsage = EWeaponUsage::None;

	/** 等级需求 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备", meta = (DisplayName = "等级需求", ClampMin = "1"))
	int32 LevelRequirement = 1;

	// ===== 五维加成 =====
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

	/** 是否为双手武器 */
	bool bIsTwoHanded() const { return Slot == EEquipmentSlot::MainHand && WeaponUsage == EWeaponUsage::TwoHand; }

	/** 是否为副手物品 */
	bool bIsOffHand() const { return Slot == EEquipmentSlot::OffHand; }
};
