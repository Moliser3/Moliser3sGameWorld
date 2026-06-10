// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AIController.h"
#include "Component/Attribute/AttributeComponent.h"
#include "Component/Damage/DamageCalculatorComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 不让控制器控制角色的旋转（让移动组件控制）
	bUseControllerRotationYaw = false;

	// 创建属性组件
	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));

	// 创建伤害计算组件
	DamageCalculator = CreateDefaultSubobject<UDamageCalculatorComponent>(TEXT("DamageCalculator"));
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