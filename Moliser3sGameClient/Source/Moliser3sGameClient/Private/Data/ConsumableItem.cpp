#include "Data/ConsumableItem.h"
#include "Component/Attribute/AttributeComponent.h"
#include "GameFramework/Actor.h"

UConsumableItem::UConsumableItem()
{
	ItemCategory = EItemCategory::Consumable;
}

void UConsumableItem::Use_Implementation(AActor* User)
{
	if (!User) return;

	UAttributeComponent* AttrComp = User->FindComponentByClass<UAttributeComponent>();
	if (!AttrComp) return;

	switch (EffectType)
	{
	case EConsumableEffectType::HealHP:
		AttrComp->Heal(EffectValue);
		break;

	case EConsumableEffectType::RestoreMP:
		AttrComp->RestoreMana(EffectValue);
		break;

	case EConsumableEffectType::Buff:
		break;
	}
}
