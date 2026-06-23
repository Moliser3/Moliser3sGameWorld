#include "Skill/SkillBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

void USkillBase::Execute(AActor* Instigator) {}
void USkillBase::OnWindupUpdate(AActor* Instigator, float DeltaTime) {}
void USkillBase::OnExecute(AActor* Instigator) {}
void USkillBase::OnRecoveryUpdate(AActor* Instigator, float DeltaTime) {}
void USkillBase::OnInterrupt(AActor* Instigator) {}

void USkillBase::PlaySkillMontage(AActor* Instigator)
{
	if (!Instigator || !Stages.IsValidIndex(CurrentStage) || !Stages[CurrentStage].SkillMontage)
	{
		return;
	}

	ACharacter* Char = Cast<ACharacter>(Instigator);
	if (!Char)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Char->GetMesh();
	if (!Mesh)
	{
		return;
	}

	UAnimInstance* AnimInst = Mesh->GetAnimInstance();
	if (!AnimInst)
	{
		return;
	}

	AnimInst->Montage_Stop(0.2f);
	AnimInst->Montage_Play(Stages[CurrentStage].SkillMontage, 1.0f);
}

float USkillBase::GetWindupTime() const
{
	return Stages.IsValidIndex(CurrentStage) ? Stages[CurrentStage].WindupTime : 0.3f;
}

float USkillBase::GetRecoveryTime() const
{
	return Stages.IsValidIndex(CurrentStage) ? Stages[CurrentStage].RecoveryTime : 0.5f;
}

float USkillBase::GetCustomLinkTime() const
{
	return Stages.IsValidIndex(CurrentStage) ? Stages[CurrentStage].CustomLinkTime : 0.2f;
}

float USkillBase::GetSkillRange() const
{
	return Stages.IsValidIndex(CurrentStage) ? Stages[CurrentStage].SkillRange : 50.0f;
}
