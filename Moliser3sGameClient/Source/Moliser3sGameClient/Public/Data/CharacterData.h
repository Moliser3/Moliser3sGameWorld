#pragma once

#include "CoreMinimal.h"
#include "Data/DataDefinitions.h"
#include "CharacterData.generated.h"

/**
 * 角色核心数据
 * 五行 → 五维属性 → 派生战斗属性
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

	// ===== 五行根基值 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五行", meta = (ClampMin = "0", DisplayName = "金"))
	int32 Jin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五行", meta = (ClampMin = "0", DisplayName = "木"))
	int32 Mu = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五行", meta = (ClampMin = "0", DisplayName = "水"))
	int32 Shui = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五行", meta = (ClampMin = "0", DisplayName = "火"))
	int32 Huo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "五行", meta = (ClampMin = "0", DisplayName = "土"))
	int32 Tu = 0;

	// ===== 固定战斗参数（不由五行派生） =====
	/** 暴击倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "战斗参数", meta = (ClampMin = "1.0"))
	float CritMultiplier = 2.0f;

	/** 百分比伤害减免（0~1） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "战斗参数", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageReduction = 0.1f;

	// ===== 五维属性计算 =====
	/** 劲力 = 金 × 100% + 土 × 30%（土生金） */
	float GetJinLi()  const { return Jin * 1.0f + Tu * 0.3f; }

	/** 气血 = 木 × 100% + 水 × 30%（水生木） */
	float GetQiXue()  const { return Mu * 1.0f + Shui * 0.3f; }

	/** 内息 = 水 × 100% + 金 × 30%（金生水） */
	float GetNeiXi()  const { return Shui * 1.0f + Jin * 0.3f; }

	/** 身法 = 火 × 100% + 木 × 30%（木生火） */
	float GetShenFa() const { return Huo * 1.0f + Mu * 0.3f; }

	/** 体魄 = 土 × 100% + 火 × 30%（火生土） */
	float GetTiPo()   const { return Tu * 1.0f + Huo * 0.3f; }

	// ===== 派生战斗属性 =====
	float GetAttackPower()        const { return GetJinLi(); }
	float GetMaxHealth()          const { return GetQiXue() * 10.0f; }
	float GetMaxMana()            const { return GetNeiXi() * 10.0f; }
	float GetHealthRegen()        const { return GetTiPo() * 0.5f; }
	float GetManaRegen()          const { return GetTiPo() * 0.3f; }
	float GetExternalDefense()    const { return GetTiPo() * 1.0f + GetJinLi() * 0.3f; }
	float GetInternalDefense()    const { return GetTiPo() * 1.0f + GetNeiXi() * 0.3f; }

	/** 获取最高的五行值对应的 EWuxing */
	EWuXing GetDominantWuXing() const
	{
		int32 MaxVal = Jin;
		EWuXing Result = EWuXing::Jin;
		if (Mu > MaxVal) { MaxVal = Mu; Result = EWuXing::Mu; }
		if (Shui > MaxVal) { MaxVal = Shui; Result = EWuXing::Shui; }
		if (Huo > MaxVal) { MaxVal = Huo; Result = EWuXing::Huo; }
		if (Tu > MaxVal) { Result = EWuXing::Tu; }
		return Result;
	}

	/** 移速加成百分比（0~100） */
	float GetSpeedBonusPct() const { return FMath::Min(GetShenFa() * 0.5f, 100.0f); }

	/** 闪避率百分比（0~50） */
	float GetDodgeRatePct()  const { return FMath::Min(GetShenFa() * 0.5f, 50.0f); }

	/** 暴击率百分比（0~50） */
	float GetCritRatePct()   const { return FMath::Min(GetShenFa() * 0.5f, 50.0f); }

	// ===== 装备加成接口 =====
	void AddEquipmentBonus(int32 InJin, int32 InMu, int32 InShui, int32 InHuo, int32 InTu)
	{
		Jin += InJin; Mu += InMu; Shui += InShui; Huo += InHuo; Tu += InTu;
	}

	void RemoveEquipmentBonus(int32 InJin, int32 InMu, int32 InShui, int32 InHuo, int32 InTu)
	{
		Jin = FMath::Max(0, Jin - InJin);
		Mu = FMath::Max(0, Mu - InMu);
		Shui = FMath::Max(0, Shui - InShui);
		Huo = FMath::Max(0, Huo - InHuo);
		Tu = FMath::Max(0, Tu - InTu);
	}
};
