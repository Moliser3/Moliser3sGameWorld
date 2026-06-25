// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Component/Attribute/AttributeComponent.h"
#include "Component/Damage/DamageCalculatorComponent.h"
#include "Component/Equipment/EquipmentComponent.h"
#include "Component/Inventory/InventoryComponent.h"
#include "Data/EquipItem.h"
#include "DebugHelper.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 不让控制器控制角色的旋转（让移动组件控制）
	bUseControllerRotationYaw = false;

	// 创建属性组件
	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));

	// 创建伤害计算组件
	DamageCalculator = CreateDefaultSubobject<UDamageCalculatorComponent>(TEXT("DamageCalculator"));

	// 创建设备管理组件
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));

	// 创建背包组件
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void ABaseCharacter::MoveToLocation(const FVector& DestLocation)
{
	AController* MyController = GetController();
	if (!MyController)
	{
		return;
	}

	// 使用 SimpleMoveToLocation 自动处理 NavMesh 寻路
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyController, DestLocation);
}

void ABaseCharacter::StopMovement()
{
	// 停止移动
	if (AController* OwnerController = GetController())
	{
		OwnerController->StopMovement();
	}
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UAttributeComponent* Attr = GetAttributeComponent())
	{
		const FCharacterCoreData& Data = Attr->GetCharacterData();

		FString ActorName = GetName();

		UE_LOG(LogTemp, Warning, TEXT("======================================"));
		UE_LOG(LogTemp, Warning, TEXT("  [%s] Base Attributes"), *ActorName);
		UE_LOG(LogTemp, Warning, TEXT("======================================"));
		UE_LOG(LogTemp, Warning, TEXT("  Five: Jin=%d Mu=%d Shui=%d Huo=%d Tu=%d"),
			Data.Jin, Data.Mu, Data.Shui, Data.Huo, Data.Tu);
		UE_LOG(LogTemp, Warning, TEXT("  JinLi=%.1f QiXue=%.1f NeiXi=%.1f ShenFa=%.1f TiPo=%.1f"),
			Data.GetJinLi(), Data.GetQiXue(), Data.GetNeiXi(), Data.GetShenFa(), Data.GetTiPo());
		UE_LOG(LogTemp, Warning, TEXT("  Atk=%.0f HP=%.0f/%.0f MP=%.0f/%.0f"),
			Data.GetAttackPower(), Attr->GetHealth(), Data.GetMaxHealth(), Attr->GetMana(), Data.GetMaxMana());
		UE_LOG(LogTemp, Warning, TEXT("  HP_regen=%.1f/s MP_regen=%.1f/s ExtDef=%.1f IntDef=%.1f"),
			Data.GetHealthRegen(), Data.GetManaRegen(), Data.GetExternalDefense(), Data.GetInternalDefense());
		UE_LOG(LogTemp, Warning, TEXT("  Spd=+%.0f%% Dodge=%.1f%% Crit=%.1f%%"),
			Data.GetSpeedBonusPct(), Data.GetDodgeRatePct(), Data.GetCritRatePct());
		UE_LOG(LogTemp, Warning, TEXT("======================================"));
	}

	// ============================================================
	// 【Debug 装备测试 — 上线前需删除】
	// ============================================================
	auto TryEquip = [&](UEquipItem* Item, EEquipmentSlot Slot)
	{
		if (Item)
		{
			Item->Slot = Slot;
			GetEquipmentComponent()->EquipItem(Item);
		}
	};

	TryEquip(HelmetItem,    EEquipmentSlot::Helmet);
	TryEquip(ShouldersItem, EEquipmentSlot::Shoulders);
	TryEquip(ChestItem,     EEquipmentSlot::Chest);
	TryEquip(BracersItem,   EEquipmentSlot::Bracers);
	TryEquip(GlovesItem,    EEquipmentSlot::Gloves);
	TryEquip(BeltItem,      EEquipmentSlot::Belt);
	TryEquip(PantsItem,     EEquipmentSlot::Pants);
	TryEquip(BootsItem,     EEquipmentSlot::Boots);
	TryEquip(CloakItem,     EEquipmentSlot::Cloak);
	TryEquip(AmuletItem,    EEquipmentSlot::Amulet);
	TryEquip(Ring1Item,     EEquipmentSlot::Ring1);
	TryEquip(Ring2Item,     EEquipmentSlot::Ring2);
	TryEquip(MainHandItem,  EEquipmentSlot::MainHand);
	TryEquip(OffHandItem,   EEquipmentSlot::OffHand);

	// ============================================================
	// 【Debug 背包测试 — 上线前需删除】
	// ============================================================
	InventoryComponent->BeginBatch();
	for (UItemBase* Item : TestInventoryItems)
	{
		if (Item)
		{
			InventoryComponent->AddItem(Item);
		}
	}
	InventoryComponent->EndBatch();
}