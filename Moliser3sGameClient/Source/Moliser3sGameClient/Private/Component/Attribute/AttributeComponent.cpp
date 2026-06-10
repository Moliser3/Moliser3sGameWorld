// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Attribute/AttributeComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UAttributeComponent::UAttributeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::TakeDamage(float DamageAmount, AActor* Instigator)
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

    // 在屏幕上输出伤害日志，存在2秒
    if (GEngine && Instigator)
    {
        FString InstigatorName = Instigator->GetName();
        FString VictimName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
        FString LogMsg = FString::Printf(TEXT("[Damage] %s -> %s : %.0f damage, %.0f HP remaining"),
            *InstigatorName, *VictimName, DamageAmount, Health);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, LogMsg);
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