#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "GamePlayerState.h"
#include "PlayerCharacter.generated.h"

class UFacingComponent;
class USkillSystemComponent;
class UCameraComponent;

UCLASS(Blueprintable)
class MOLISER3SGAMECLIENT_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	UFUNCTION(BlueprintPure, Category = "Components")
	UFacingComponent* GetFacingComponent() const { return FacingComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	USkillSystemComponent* GetSkillSystem() const { return SkillSystemComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UCameraComponent* GetCamera() const { return CameraComponent; }

	UFUNCTION()
	void OnCombatStateChanged(ECombatState NewCombat);

	UFUNCTION()
	void OnActionStateChanged(EActionState NewAction);

protected:
	virtual void BeginPlay() override;

	void ApplyFacingForState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFacingComponent> FacingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkillSystemComponent> SkillSystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom", meta = (ClampMin = "0.0"))
	float BattlePerceptionRange = 1500.0f;

public:
	UFUNCTION(BlueprintPure, Category = "Facing")
	float GetBattlePerceptionRange() const { return BattlePerceptionRange; }
};
