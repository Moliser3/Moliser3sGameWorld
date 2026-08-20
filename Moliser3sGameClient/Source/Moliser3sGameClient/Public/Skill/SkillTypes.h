#pragma once

#include "CoreMinimal.h"
#include "SkillTypes.generated.h"

UENUM(BlueprintType)
enum class ESkillCategory : uint8
{
	Attack   UMETA(DisplayName = "攻击"),
	Movement UMETA(DisplayName = "位移"),
	Utility  UMETA(DisplayName = "辅助"),
	Hybrid   UMETA(DisplayName = "复合")
};

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	None UMETA(DisplayName = "无")
};

class UAnimMontage;

USTRUCT(BlueprintType)
struct FSkillStage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", DisplayName = "前摇"))
	float WindupTime = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", DisplayName = "后摇"))
	float RecoveryTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", DisplayName = "衔接时间"))
	float CustomLinkTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "蒙太奇")
	TObjectPtr<UAnimMontage> SkillMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "蒙太奇槽位")
	FName MontageSlotName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", DisplayName = "基础伤害"))
	float BaseDamage = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", DisplayName = "扇形半角"))
	float HalfAngleDeg = 22.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", DisplayName = "最大高度差"))
	float MaxZDiff = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", DisplayName = "技能范围"))
	float SkillRange = 50.0f;

	/** 该阶段的外伤占比（0~1），剩余为内伤 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "外伤占比", ClampMin = "0.0", ClampMax = "1.0"))
	float ExternalDamageRatio = 0.7f;
};
