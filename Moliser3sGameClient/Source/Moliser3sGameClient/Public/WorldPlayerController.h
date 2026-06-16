#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WorldPlayerController.generated.h"

class UClickDetectionComponent;
class UInputMappingContext;
class UCameraControllerComponent;

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Default UMETA(DisplayName = "默认状态"),
	Battle  UMETA(DisplayName = "战斗状态")
};

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

	UFUNCTION(BlueprintPure, Category = "Click")
	FVector GetLastClickTarget() const { return LastClickTarget; }

	UFUNCTION(BlueprintCallable, Category = "Click")
	void SetLastClickTarget(const FVector& NewTarget) { LastClickTarget = NewTarget; }

	UFUNCTION(BlueprintPure, Category = "State")
	EPlayerState GetPlayerState() const { return CurrentPlayerState; }

protected:
	virtual void BeginPlay() override;

	void SetPlayerState(EPlayerState NewState);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Click")
	TObjectPtr<UClickDetectionComponent> ClickDetectionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraControllerComponent> CameraControllerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Click")
	FVector LastClickTarget = FVector::ZeroVector;

	bool bPendingAttack = false;
	float PendingMaxRange = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EPlayerState CurrentPlayerState = EPlayerState::Default;

	bool bPreviousSkillActive = false;
	bool bPendingRestoreAiming = false;
	bool bDefending = false;
};
