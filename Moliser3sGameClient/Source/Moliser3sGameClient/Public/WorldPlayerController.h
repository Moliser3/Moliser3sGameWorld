#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GamePlayerState.h"
#include "WorldPlayerController.generated.h"

class UClickDetectionComponent;
class UInputMappingContext;
class UCameraControllerComponent;

UCLASS()
class MOLISER3SGAMECLIENT_API AWorldPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWorldPlayerController();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Click")
	UClickDetectionComponent* GetClickDetectionComponent() const { return ClickDetectionComponent; }

	UFUNCTION(BlueprintPure, Category = "Camera")
	UCameraControllerComponent* GetCameraController() const { return CameraControllerComponent; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnLeftMouseClick();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnRightMouseClick();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnAltPressed();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnAltReleased();

	UFUNCTION(BlueprintCallable, Category = "快捷栏")
	void OnQuickSlotKeyPressed(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Click")
	FVector GetLastClickTarget() const { return LastClickTarget; }

	UFUNCTION(BlueprintCallable, Category = "Click")
	void SetLastClickTarget(const FVector& NewTarget) { LastClickTarget = NewTarget; }

	UFUNCTION(BlueprintPure, Category = "State")
	ECombatState GetCombatState() const { return CurrentCombatState; }

	UFUNCTION(BlueprintPure, Category = "State")
	EActionState GetActionState() const { return CurrentActionState; }

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsMouseOverUI() const;

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnCombatStateChanged OnCombatStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnActionStateChanged OnActionStateChanged;

protected:
	virtual void BeginPlay() override;

	void SetCombatState(ECombatState NewState);
	void SetActionState(EActionState NewState);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Click")
	TObjectPtr<UClickDetectionComponent> ClickDetectionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraControllerComponent> CameraControllerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Click")
	FVector LastClickTarget = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ECombatState CurrentCombatState = ECombatState::Default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EActionState CurrentActionState = EActionState::Idle;

	float RunStartTime = 0.0f;
	bool bPendingAttack = false;
	float PendingMaxRange = 0.0f;
	bool bDefending = false;
};
