#pragma once

#include "CoreMinimal.h"
#include "Skill/SkillBase.h"
#include "DamageSkillBase.generated.h"

/**
 * 伤害技能基类
 * 所有造成伤害的技能继承此类。
 * ApplyDamage 在 OnExecute() 中手动调用（前摇→后摇切换帧）。
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UDamageSkillBase : public USkillBase
{
	GENERATED_BODY()

public:
	/**
	 * 应用伤害（由 OnExecute 中调用）
	 * 子类在此方法中实现伤害逻辑
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void ApplyDamage(AActor* Instigator);
};
