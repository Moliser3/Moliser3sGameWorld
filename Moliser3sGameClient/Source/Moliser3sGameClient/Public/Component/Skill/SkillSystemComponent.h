#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill/SkillTypes.h"
#include "SkillSystemComponent.generated.h"

class USkillBase;

UENUM(BlueprintType)
enum class ESkillPhase : uint8
{
	Idle        UMETA(DisplayName = "空闲"),
	Windup      UMETA(DisplayName = "前摇"),
	Recovery    UMETA(DisplayName = "后摇"),
	LinkWindow  UMETA(DisplayName = "衔接")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API USkillSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillSystemComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ActivateLeft();

	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ActivateRight();

	UFUNCTION(BlueprintPure, Category = "Skill")
	float GetMaxSkillRange() const;

	UFUNCTION(BlueprintPure, Category = "Skill")
	USkillBase* GetCurrentSkill() const { return CurrentSkill; }

	UFUNCTION(BlueprintPure, Category = "Skill")
	ESkillPhase GetSkillPhase() const { return SkillPhase; }

protected:
	void ExecuteSkillFromGroup(TArray<TObjectPtr<USkillBase>>& Group, int32& Index,
	                           ESkillType& LastType, int32& StageForType);

	/** 左键技能组 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "LeftSkill")
	TArray<TObjectPtr<USkillBase>> LeftSkillGroup;

	/** 右键技能组 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "RightSkill")
	TArray<TObjectPtr<USkillBase>> RightSkillGroup;

	UPROPERTY(VisibleInstanceOnly, Category = "Skill")
	ESkillPhase SkillPhase = ESkillPhase::Idle;

	float PhaseStartTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Skill")
	TObjectPtr<USkillBase> CurrentSkill = nullptr;

	int32 LeftGroupIndex = 0;
	int32 RightGroupIndex = 0;

	ESkillType LeftLastSkillType = ESkillType::None;
	int32 LeftStageForType = 0;

	ESkillType RightLastSkillType = ESkillType::None;
	int32 RightStageForType = 0;

	float CachedLinkDuration = 0.0f;
};
