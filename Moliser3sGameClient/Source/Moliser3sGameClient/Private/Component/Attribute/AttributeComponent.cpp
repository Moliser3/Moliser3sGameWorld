#include "Component/Attribute/AttributeComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bInitializeFull)
	{
		Health = GetMaxHealth();
		Mana = GetMaxMana();
	}
}

void UAttributeComponent::SetCharacterData(const FCharacterCoreData& NewData)
{
	CharacterData = NewData;

	if (bInitializeFull)
	{
		Health = CharacterData.GetMaxHealth();
		Mana = CharacterData.GetMaxMana();
	}
	else
	{
		Health = FMath::Min(Health, CharacterData.GetMaxHealth());
		Mana = FMath::Min(Mana, CharacterData.GetMaxMana());
	}
}

void UAttributeComponent::TakeDamage(float DamageAmount, AActor* Instigator)
{
	if (DamageAmount <= 0.0f)
	{
		return;
	}

	float OldHealth = Health;
	Health = FMath::Clamp(Health - DamageAmount, 0.0f, GetMaxHealth());

	if (OldHealth != Health)
	{
		OnHealthChanged.Broadcast(OldHealth, Health);
	}

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
	Health = FMath::Clamp(Health + HealAmount, 0.0f, GetMaxHealth());

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
	Mana = FMath::Clamp(Mana - Cost, 0.0f, GetMaxMana());

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
	Mana = FMath::Clamp(Mana + Amount, 0.0f, GetMaxMana());

	if (OldMana != Mana)
	{
		OnManaChanged.Broadcast(OldMana, Mana);
	}
}

void UAttributeComponent::RestoreFullHealth()
{
	float OldHealth = Health;
	Health = GetMaxHealth();

	if (OldHealth != Health)
	{
		OnHealthChanged.Broadcast(OldHealth, Health);
	}
}

void UAttributeComponent::RestoreFullMana()
{
	float OldMana = Mana;
	Mana = GetMaxMana();

	if (OldMana != Mana)
	{
		OnManaChanged.Broadcast(OldMana, Mana);
	}
}

void UAttributeComponent::SyncToMaxValues()
{
	RestoreFullHealth();
	RestoreFullMana();
}