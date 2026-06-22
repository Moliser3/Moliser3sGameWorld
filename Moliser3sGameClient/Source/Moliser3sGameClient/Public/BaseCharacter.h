#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UAttributeComponent;
class UDamageCalculatorComponent;
class UEquipmentComponent;
class UInventoryComponent;
class UItemBase;
class UEquipItem;

UCLASS(Blueprintable)
class MOLISER3SGAMECLIENT_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveToLocation(const FVector& DestLocation);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopMovement();

	UFUNCTION(BlueprintPure, Category = "Animation")
	float GetSpeed() const { return GetVelocity().Length(); }

	UFUNCTION(BlueprintPure, Category = "Components")
	UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UDamageCalculatorComponent* GetDamageCalculator() const { return DamageCalculator; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UInventoryComponent* GetInventory() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetWalkSpeed() const { return WalkSpeed; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetRunSpeed() const { return RunSpeed; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDamageCalculatorComponent> DamageCalculator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEquipmentComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom", meta = (ClampMin = "0.0"))
	float WalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom", meta = (ClampMin = "0.0"))
	float RunSpeed = 600.0f;

	// ============================================================
	// 【Debug 装备测试 — 上线前需删除】
	// 直接给角色穿戴装备，后期由背包系统替代
	// ============================================================
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "头盔"))
	TObjectPtr<UEquipItem> HelmetItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "肩甲"))
	TObjectPtr<UEquipItem> ShouldersItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "胸甲"))
	TObjectPtr<UEquipItem> ChestItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "护腕"))
	TObjectPtr<UEquipItem> BracersItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "手套"))
	TObjectPtr<UEquipItem> GlovesItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "腰带"))
	TObjectPtr<UEquipItem> BeltItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "裤子"))
	TObjectPtr<UEquipItem> PantsItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "靴子"))
	TObjectPtr<UEquipItem> BootsItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "项链"))
	TObjectPtr<UEquipItem> AmuletItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "戒指1"))
	TObjectPtr<UEquipItem> Ring1Item;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "戒指2"))
	TObjectPtr<UEquipItem> Ring2Item;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "主手武器"))
	TObjectPtr<UEquipItem> MainHandItem;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|装备测试", meta = (DisplayName = "副手"))
	TObjectPtr<UEquipItem> OffHandItem;

	// ============================================================
	// 【Debug 背包测试 — 上线前需删除】
	// 直接给背包添加物品，上线后由游戏逻辑/掉落系统填充
	// ============================================================
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "Debug|背包测试")
	TArray<TObjectPtr<UItemBase>> TestInventoryItems;
};
