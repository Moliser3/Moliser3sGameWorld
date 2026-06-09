// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Component/Attribute/AttributeComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 不让控制器控制角色的旋转（让移动组件控制）
	bUseControllerRotationYaw = false;

	// 设置移动参数
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		MoveComp->MaxWalkSpeed = MoveSpeed;
		MoveComp->bUseControllerDesiredRotation = false;
	}

	// 创建属性组件
	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
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

	// SimpleMoveToLocation 内部使用 AIController 执行 MoveToLocation
	// bUseControllerRotationYaw = false 确保角色的 Yaw 旋转不被控制器覆盖
	// bOrientRotationToMovement = true 让 Mesh 自动面朝移动方向（速度方向）
	// RotationRate = 720°/s 控制转向速度
}

void ABaseCharacter::StopMovement()
{
	// 停止移动
	if (AController* OwnerController = GetController())
	{
		OwnerController->StopMovement();
	}
}
