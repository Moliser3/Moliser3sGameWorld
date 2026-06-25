#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CharacterData.h"
#include "AttributeComponent.generated.h"

/** 属性变化时广播的事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, OldHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChangedDelegate, float, OldMana, float, NewMana);

/**
 * 角色属性组件
 * 管理五行根基值、运行时血量/法力、战斗属性
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	virtual void BeginPlay() override;

	// ===== 角色核心数据（五行 → 五维 → 派生属性） =====
	UFUNCTION(BlueprintPure, Category = "Attributes")
	const FCharacterCoreData& GetCharacterData() const { return CharacterData; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetCharacterData(const FCharacterCoreData& NewData);

	// ===== 血量 =====
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetMaxHealth() const { return CharacterData.GetMaxHealth(); }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetHealthPercent() const { return GetMaxHealth() > 0.0f ? Health / GetMaxHealth() : 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void TakeDamage(float DamageAmount, AActor* Instigator = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void Heal(float HealAmount);

	// ===== 法力 =====
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetMana() const { return Mana; }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetMaxMana() const { return CharacterData.GetMaxMana(); }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetManaPercent() const { return GetMaxMana() > 0.0f ? Mana / GetMaxMana() : 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ConsumeMana(float Cost);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void RestoreMana(float Amount);

	// ===== 攻击属性（由五行派生） =====
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetBaseDamage() const { return CharacterData.GetAttackPower(); }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCritRate() const { return CharacterData.GetCritRatePct() / 100.0f; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCritMultiplier() const { return CharacterData.CritMultiplier; }

	// ===== 防御属性（由五行派生） =====
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetExternalDefense() const { return CharacterData.GetExternalDefense(); }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetInternalDefense() const { return CharacterData.GetInternalDefense(); }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetDamageReduction() const { return CharacterData.DamageReduction; }

	// ===== 五维属性 =====
	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetJinLi() const { return CharacterData.GetJinLi(); }

	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetQiXue() const { return CharacterData.GetQiXue(); }

	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetNeiXi() const { return CharacterData.GetNeiXi(); }

	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetShenFa() const { return CharacterData.GetShenFa(); }

	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetTiPo() const { return CharacterData.GetTiPo(); }

	// ===== 五行派生便捷接口 =====
	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetAttackPower() const { return CharacterData.GetAttackPower(); }

	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetHealthRegen() const { return CharacterData.GetHealthRegen(); }

	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetManaRegen() const { return CharacterData.GetManaRegen(); }

	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetSpeedBonusPct() const { return CharacterData.GetSpeedBonusPct(); }

	UFUNCTION(BlueprintPure, Category = "WuXing")
	float GetDodgeRatePct() const { return CharacterData.GetDodgeRatePct(); }

	/** 恢复满血量 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void RestoreFullHealth();

	/** 恢复满法力 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void RestoreFullMana();

	/** 重新同步运行时血量/法力到上限 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SyncToMaxValues();

public:
	/** 血量变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnHealthChangedDelegate OnHealthChanged;

	/** 法力变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnManaChangedDelegate OnManaChanged;

protected:
	// ===== 五行核心数据 =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "五行核心数据")
	FCharacterCoreData CharacterData;

	// ===== 运行时状态 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "运行时状态")
	float Health = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "运行时状态")
	float Mana = 0.0f;

	/** 是否在 BeginPlay 时初始化满状态 */
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	bool bInitializeFull = true;
};