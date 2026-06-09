// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

/** 属性变化时广播的事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, OldHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChangedDelegate, float, OldMana, float, NewMana);

/**
 * 角色属性组件
 * 管理血量、法力等基础属性
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOLISER3SGAMECLIENT_API UAttributeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAttributeComponent();

    // ===== 血量 =====
    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetHealthPercent() const { return MaxHealth > 0.0f ? Health / MaxHealth : 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void Heal(float HealAmount);

    // ===== 法力 =====
    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetMana() const { return Mana; }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetMaxMana() const { return MaxMana; }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetManaPercent() const { return MaxMana > 0.0f ? Mana / MaxMana : 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void ConsumeMana(float Cost);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void RestoreMana(float Amount);

public:
    /** 血量变化事件 */
    UPROPERTY(BlueprintAssignable, Category = "Attributes")
    FOnHealthChangedDelegate OnHealthChanged;

    /** 法力变化事件 */
    UPROPERTY(BlueprintAssignable, Category = "Attributes")
    FOnManaChangedDelegate OnManaChanged;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
    float Health = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    float MaxMana = 50.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
    float Mana = 50.0f;

    /** 是否在 BeginPlay 时初始化满状态 */
    UPROPERTY(EditDefaultsOnly, Category = "Attributes")
    bool bInitializeFull = true;
};