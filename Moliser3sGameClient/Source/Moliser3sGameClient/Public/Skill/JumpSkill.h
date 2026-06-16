#pragma once

#include "CoreMinimal.h"
#include "Skill/SkillBase.h"
#include "JumpSkill.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API UJumpSkill : public USkillBase
{
	GENERATED_BODY()

public:
	UJumpSkill();

	virtual void Execute(AActor* Instigator) override;
	virtual void OnWindupUpdate(AActor* Instigator, float DeltaTime) override;
	virtual void OnExecute(AActor* Instigator) override;
	virtual void OnRecoveryUpdate(AActor* Instigator, float DeltaTime) override {}
	virtual void OnInterrupt(AActor* Instigator) override;

	/** 最大跳跃距离（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", DisplayName = "最大跳跃距离"))
	float JumpRange = 500.0f;

	/** 抛物线最高点高度（厘米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", DisplayName = "跳跃最高点高度"))
	float JumpHeight = 200.0f;

protected:
	bool IsTargetReachable(AActor* Instigator, const FVector& Target) const;

	bool bIsJumping = false;
	FVector JumpStartLoc = FVector::ZeroVector;
	FVector JumpTargetLoc = FVector::ZeroVector;
	float JumpProgress = 0.0f;
};
