#pragma once

#include "CoreMinimal.h"
#include "EquipmentData.generated.h"

/** 装备槽位枚举 - 共14个 */
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	Helmet    UMETA(DisplayName = "头盔"),
	Shoulders UMETA(DisplayName = "肩甲"),
	Chest     UMETA(DisplayName = "胸甲"),
	Bracers   UMETA(DisplayName = "护腕"),
	Gloves    UMETA(DisplayName = "手套"),
	Belt      UMETA(DisplayName = "腰带"),
	Pants     UMETA(DisplayName = "裤子"),
	Boots     UMETA(DisplayName = "靴子"),
	Amulet    UMETA(DisplayName = "项链"),
	Ring1     UMETA(DisplayName = "戒指1"),
	Ring2     UMETA(DisplayName = "戒指2"),
	MainHand  UMETA(DisplayName = "主手武器"),
	OffHand   UMETA(DisplayName = "副手")
};

/** 武器使用类型 */
UENUM(BlueprintType)
enum class EWeaponUsage : uint8
{
	OneHand UMETA(DisplayName = "单手"),
	TwoHand UMETA(DisplayName = "双手")
};

/** 物品稀有度 */
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Normal UMETA(DisplayName = "普通"),
	Magic  UMETA(DisplayName = "魔法"),
	Rare   UMETA(DisplayName = "稀有"),
	Unique UMETA(DisplayName = "独特")
};

/** 装备物品基础数据 */
USTRUCT(BlueprintType)
struct FEquipmentItemData
{
	GENERATED_BODY()

	/** 物品唯一标识 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FName ItemID = NAME_None;

	/** 显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FText DisplayName;

	/** 物品描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "基础")
	FText Description;

	/** 装备槽位 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备属性")
	EEquipmentSlot Slot = EEquipmentSlot::Helmet;

	/** 稀有度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备属性")
	EItemRarity Rarity = EItemRarity::Normal;

	/** 武器类型（非武器时无效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备属性")
	EWeaponUsage WeaponUsage = EWeaponUsage::OneHand;

	/** 等级需求 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "装备属性", meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	/** 是否为双手武器（占用主手+锁定副手） */
	bool bIsTwoHanded() const { return Slot == EEquipmentSlot::MainHand && WeaponUsage == EWeaponUsage::TwoHand; }

	/** 是否为副手物品 */
	bool bIsOffHand() const { return Slot == EEquipmentSlot::OffHand; }
};
