#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Skill/SkillTypes.h"
#include "SkillBase.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class MOLISER3SGAMECLIENT_API USkillBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void Execute(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnWindupUpdate(AActor* Instigator, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnExecute(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnRecoveryUpdate(AActor* Instigator, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void OnInterrupt(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	void PlaySkillMontage(AActor* Instigator);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "技能名称"))
	FName SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "技能类型"))
	ESkillType SkillType = ESkillType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "技能分类"))
	ESkillCategory SkillCategory = ESkillCategory::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (DisplayName = "技能阶段列表"))
	TArray<FSkillStage> Stages;

	void SetCurrentStage(int32 Index) { CurrentStage = Index; }
	int32 GetCurrentStage() const { return CurrentStage; }

	float GetWindupTime() const;
	float GetRecoveryTime() const;
	float GetCustomLinkTime() const;
	float GetSkillRange() const;

private:
	int32 CurrentStage = 0;
};
