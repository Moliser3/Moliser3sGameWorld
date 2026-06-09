// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Attribute/AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::TakeDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f)
    {
        return;
    }

    float OldHealth = Health;
    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);

    if (OldHealth != Health)
    {
        OnHealthChanged.Broadcast(OldHealth, Health);
    }
}

void UAttributeComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f)
    {
        return;
    }

    float OldHealth = Health;
    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);

    if (OldHealth != Health)
    {
        OnHealthChanged.Broadcast(OldHealth, Health);
    }
}

void UAttributeComponent::ConsumeMana(float Cost)
{
    if (Cost <= 0.0f)
    {
        return;
    }

    float OldMana = Mana;
    Mana = FMath::Clamp(Mana - Cost, 0.0f, MaxMana);

    if (OldMana != Mana)
    {
        OnManaChanged.Broadcast(OldMana, Mana);
    }
}

void UAttributeComponent::RestoreMana(float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }

    float OldMana = Mana;
    Mana = FMath::Clamp(Mana + Amount, 0.0f, MaxMana);

    if (OldMana != Mana)
    {
        OnManaChanged.Broadcast(OldMana, Mana);
    }
}