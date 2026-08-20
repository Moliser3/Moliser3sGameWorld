#pragma once

#include "CoreMinimal.h"
#include "Data/DataDefinitions.h"
#include "CharacterData.generated.h"

/**
 * 角色核心数据
 * 五维属性 → 派生战斗属性
 */
USTRUCT(BlueprintType)
struct FCharacterCoreData
{
	GENERATED_BODY()

	// ===== 等级 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "等级", meta = (ClampMin = "1"))
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "等级")
	float Experience = 0.0f;

	// ===== 五维基础属性 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五维", meta = (ClampMin = "0", DisplayName = "劲力"))
	float JinLi = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五维", meta = (ClampMin = "0", DisplayName = "气血"))
	float QiXue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五维", meta = (ClampMin = "0", DisplayName = "内息"))
	float NeiXi = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五维", meta = (ClampMin = "0", DisplayName = "身法"))
	float ShenFa = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五维", meta = (ClampMin = "0", DisplayName = "体魄"))
	float TiPo = 0.0f;

	// ===== 固定战斗参数 =====
	/** 暴击倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "战斗参数", meta = (ClampMin = "1.0"))
	float CritMultiplier = 2.0f;

	/** 百分比伤害减免（0~1） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "战斗参数", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageReduction = 0.1f;

	// ===== 五维属性访问 =====
	float GetJinLi()  const { return JinLi; }
	float GetQiXue()  const { return QiXue; }
	float GetNeiXi()  const { return NeiXi; }
	float GetShenFa() const { return ShenFa; }
	float GetTiPo()   const { return TiPo; }

	// ===== 派生战斗属性 =====
	float GetAttackPower()        const { return GetJinLi(); }
	float GetMaxHealth()          const { return GetQiXue() * 10.0f; }
	float GetMaxMana()            const { return GetNeiXi() * 10.0f; }
	float GetHealthRegen()        const { return GetTiPo() * 0.5f; }
	float GetManaRegen()          const { return GetTiPo() * 0.3f; }
	float GetExternalDefense()    const { return GetTiPo() * 1.0f + GetJinLi() * 0.3f; }
	float GetInternalDefense()    const { return GetTiPo() * 1.0f + GetNeiXi() * 0.3f; }

	/** 移速加成百分比（0~100） */
	float GetSpeedBonusPct() const { return FMath::Min(GetShenFa() * 0.5f, 100.0f); }

	/** 闪避率百分比（0~50） */
	float GetDodgeRatePct()  const { return FMath::Min(GetShenFa() * 0.5f, 50.0f); }

	/** 暴击率百分比（0~50） */
	float GetCritRatePct()   const { return FMath::Min(GetShenFa() * 0.5f, 50.0f); }

	// ===== 装备加成接口 =====
	void AddEquipmentBonus(float InJinLi, float InQiXue, float InNeiXi, float InShenFa, float InTiPo)
	{
		JinLi += InJinLi; QiXue += InQiXue; NeiXi += InNeiXi; ShenFa += InShenFa; TiPo += InTiPo;
	}

	void RemoveEquipmentBonus(float InJinLi, float InQiXue, float InNeiXi, float InShenFa, float InTiPo)
	{
		JinLi = FMath::Max(0.0f, JinLi - InJinLi);
		QiXue = FMath::Max(0.0f, QiXue - InQiXue);
		NeiXi = FMath::Max(0.0f, NeiXi - InNeiXi);
		ShenFa = FMath::Max(0.0f, ShenFa - InShenFa);
		TiPo = FMath::Max(0.0f, TiPo - InTiPo);
	}
};