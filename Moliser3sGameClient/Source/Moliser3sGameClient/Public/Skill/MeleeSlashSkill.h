#pragma once

#include "CoreMinimal.h"
#include "Skill/DamageSkillBase.h"
#include "MeleeSlashSkill.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UMeleeSlashSkill : public UDamageSkillBase
{
	GENERATED_BODY()

public:
	virtual void Execute(AActor* Instigator) override;
	virtual void OnExecute(AActor* Instigator) override;
	virtual void ApplyDamage(AActor* Instigator) override;
};
