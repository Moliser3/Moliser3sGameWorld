#pragma once

#include "CoreMinimal.h"
#include "SkillTypes.generated.h"

UENUM(BlueprintType)
enum class ESkillCategory : uint8
{
	Attack   UMETA(DisplayName = "攻击"),   // 普通攻击技能，受距离约束
	Movement UMETA(DisplayName = "位移"),   // 纯位移（如跳跃），点击即触发
	Utility  UMETA(DisplayName = "辅助"),   // 辅助技能（如格挡/增益），点击即触发
	Hybrid   UMETA(DisplayName = "复合")    // 位移+伤害（如冲锋），对敌人受距离约束，点地直接执行
};
