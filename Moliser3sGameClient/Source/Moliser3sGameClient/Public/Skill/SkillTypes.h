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
};
