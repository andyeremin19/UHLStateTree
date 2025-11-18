// Pavel Penkov 2025 All Rights Reserved.

#include "Tasks/UHLSTTask_PlayAnimMontage.h"

#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "StateTreeLinker.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UHLSTTask_PlayAnimMontage)

#define LOCTEXT_NAMESPACE "UHLSTTask_PlayAnimMontage"

USkeletalMeshComponent* FUHLSTTask_PlayAnimMontage::ResolveMesh(const FInstanceDataType& InstanceData) const
{
	if (InstanceData.CustomMesh)
	{
		return InstanceData.CustomMesh;
	}
	return InstanceData.Character ? InstanceData.Character->GetMesh() : nullptr;
}

bool FUHLSTTask_PlayAnimMontage::PlayMontage(
	FStateTreeExecutionContext& Context,
	FInstanceDataType& InstanceData,
	USkeletalMeshComponent* Mesh) const
{
	bool bPlayedSuccessfully = false;
	float MontageLength = -1.0f;
	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	// If a starting section is provided, jump to it; otherwise use starting position
	if (Mesh == InstanceData.Character->GetMesh())
	{
		// Playing on Character's main mesh will replicate to simulated proxies when executed on the server
		MontageLength = InstanceData.Character->PlayAnimMontage(InstanceData.AnimMontage, InstanceData.PlayRate);
		if (InstanceData.StartingSection != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(InstanceData.StartingSection, InstanceData.AnimMontage);
		}
		else if (InstanceData.StartingPosition > 0.0f)
		{
			AnimInstance->Montage_SetPosition(InstanceData.AnimMontage, InstanceData.StartingPosition);
		}
	}
	else
	{
		MontageLength = AnimInstance->Montage_Play(InstanceData.AnimMontage, InstanceData.PlayRate, EMontagePlayReturnType::MontageLength, InstanceData.StartingSection != NAME_None ? 0.0f : InstanceData.StartingPosition, true);
		if (InstanceData.StartingSection != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(InstanceData.StartingSection, InstanceData.AnimMontage);
		}
	}

	const FStateTreeWeakExecutionContext WeakContext = Context.MakeWeakExecutionContext();

	Mesh->GetWorld()->GetTimerManager().SetTimerForNextTick([WeakContext, AnimInstance]()
	{
		if (!AnimInstance)
		{
			return;
		}

		FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
		if (!StrongContext.IsValid())
		{
			return;
		}

		FUHLSTTask_PlayAnimMontage::FInstanceDataType* InstanceDataPtr = StrongContext.GetInstanceDataPtr<FUHLSTTask_PlayAnimMontage::FInstanceDataType>();
		if (!InstanceDataPtr)
		{
			return;
		}

		const bool bBindBlendOut = InstanceDataPtr->bFinishTaskOnBlendOut;
		const bool bBindEnd = InstanceDataPtr->bFinishTaskOnCompleted || InstanceDataPtr->bFinishTaskOnInterrupted;
		if (!bBindBlendOut && !bBindEnd)
		{
			return;
		}

		InstanceDataPtr->BoundAnimInstance = AnimInstance;

		if (bBindBlendOut)
		{
			FOnMontageBlendingOutStarted BlendOutDelegate;
			BlendOutDelegate.BindLambda(
				[WeakContext](UAnimMontage* Montage, bool bInterrupted)
				{
					FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
					if (!StrongContext.IsValid())
					{
						return;
					}

					FUHLSTTask_PlayAnimMontage::FInstanceDataType* InstanceDataPtr = StrongContext.GetInstanceDataPtr<FUHLSTTask_PlayAnimMontage::FInstanceDataType>();
					if (!InstanceDataPtr)
					{
						return;
					}

					if (Montage != InstanceDataPtr->AnimMontage)
					{
						return;
					}

					if (InstanceDataPtr->bBlendOutTriggered)
					{
						return;
					}

					InstanceDataPtr->bBlendOutTriggered = true;
					InstanceDataPtr->bInterruptedTriggered |= bInterrupted;
					InstanceDataPtr->bCompletedTriggered |= !bInterrupted;

					const bool bShouldSucceed = InstanceDataPtr->bSucceededResult && !bInterrupted;
					const EStateTreeFinishTaskType FinishType = bShouldSucceed ? EStateTreeFinishTaskType::Succeeded : EStateTreeFinishTaskType::Failed;
					WeakContext.FinishTask(FinishType);
				}
			);

			AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, InstanceDataPtr->AnimMontage);
		}

		if (bBindEnd)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindLambda(
				[WeakContext](UAnimMontage* Montage, bool bInterrupted)
				{
					FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
					if (!StrongContext.IsValid())
					{
						return;
					}

					FUHLSTTask_PlayAnimMontage::FInstanceDataType* InstanceDataPtr = StrongContext.GetInstanceDataPtr<FUHLSTTask_PlayAnimMontage::FInstanceDataType>();
					if (!InstanceDataPtr)
					{
						return;
					}

					if (Montage != InstanceDataPtr->AnimMontage)
					{
						return;
					}

					if (InstanceDataPtr->bBlendOutTriggered)
					{
						return;
					}

					if (bInterrupted)
					{
						if (!InstanceDataPtr->bFinishTaskOnInterrupted || InstanceDataPtr->bInterruptedTriggered)
						{
							return;
						}

						InstanceDataPtr->bInterruptedTriggered = true;

						const bool bShouldSucceed = InstanceDataPtr->bSucceededResult && !bInterrupted;
						const EStateTreeFinishTaskType FinishType = bShouldSucceed ? EStateTreeFinishTaskType::Succeeded : EStateTreeFinishTaskType::Failed;
						WeakContext.FinishTask(FinishType);
						return;
					}

					if (!InstanceDataPtr->bFinishTaskOnCompleted || InstanceDataPtr->bCompletedTriggered)
					{
						return;
					}

					InstanceDataPtr->bCompletedTriggered = true;

					const bool bShouldSucceed = InstanceDataPtr->bSucceededResult;
					const EStateTreeFinishTaskType FinishType = bShouldSucceed ? EStateTreeFinishTaskType::Succeeded : EStateTreeFinishTaskType::Failed;
					WeakContext.FinishTask(FinishType);
				}
			);

			AnimInstance->Montage_SetEndDelegate(EndDelegate, InstanceDataPtr->AnimMontage);
		}
	});

	bPlayedSuccessfully = (MontageLength > 0.f);
	return bPlayedSuccessfully;
}

EStateTreeRunStatus FUHLSTTask_PlayAnimMontage::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bCompletedTriggered = false;
	InstanceData.bInterruptedTriggered = false;
	InstanceData.bBlendOutTriggered = false;
	InstanceData.BoundAnimInstance.Reset();
	if (!InstanceData.Character || !InstanceData.AnimMontage)
	{
		return EStateTreeRunStatus::Failed;
	}

	USkeletalMeshComponent* Mesh = ResolveMesh(InstanceData);
	if (!Mesh)
	{
		return EStateTreeRunStatus::Failed;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return EStateTreeRunStatus::Failed;
	}

    if (InstanceData.bShouldStopAllMontages)
	{
		// Use character API when operating on the main mesh for better replication behavior
		if (Mesh == InstanceData.Character->GetMesh())
		{
			InstanceData.Character->StopAnimMontage();
		}
		else
		{
            AnimInstance->StopAllMontages(0.25f);
		}
	}
	
	bool bSuccessPlayMontage = PlayMontage(Context, InstanceData, Mesh);

	return EStateTreeRunStatus::Running;
}

void FUHLSTTask_PlayAnimMontage::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (UAnimInstance* AnimInstance = InstanceData.BoundAnimInstance.Get())
	{
		// Clear bound delegates to avoid dangling refs
		FOnMontageEnded EmptyEnded;
		AnimInstance->Montage_SetEndDelegate(EmptyEnded, InstanceData.AnimMontage);
		FOnMontageBlendingOutStarted EmptyBlendOut;
		AnimInstance->Montage_SetBlendingOutDelegate(EmptyBlendOut, InstanceData.AnimMontage);
	}
}

#if WITH_EDITOR
FText FUHLSTTask_PlayAnimMontage::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
	check(InstanceData);

	FString MontageStr;
	const FPropertyBindingPath MontagePath(ID, GET_MEMBER_NAME_CHECKED(FUHLSTTask_PlayAnimMontageInstanceData, AnimMontage));
	FText MontageBinding = BindingLookup.GetBindingSourceDisplayName(MontagePath);
	if (!MontageBinding.IsEmpty())
	{
		MontageStr = FString::Printf(TEXT("Bound: %s"), *MontageBinding.ToString());
	}
	else
	{
		MontageStr = InstanceData->AnimMontage ? InstanceData->AnimMontage->GetName() : TEXT("None");
	}

    TArray<FString> Parts;
    Parts.Add(FString::Printf(TEXT("Play AnimMontage '%s'"), *MontageStr));

    if (InstanceData->CustomMesh)
    {
        Parts.Add(TEXT("Mesh: CustomMesh"));
    }

    if (!FMath::IsNearlyEqual(InstanceData->PlayRate, 1.0f))
    {
        Parts.Add(FString::Printf(TEXT("Rate %.2f"), InstanceData->PlayRate));
    }

    if (InstanceData->StartingSection != NAME_None)
    {
        Parts.Add(FString::Printf(TEXT("Section '%s'"), *InstanceData->StartingSection.ToString()));
    }
    else if (!FMath::IsNearlyZero(InstanceData->StartingPosition))
    {
        Parts.Add(FString::Printf(TEXT("Position %.2f"), InstanceData->StartingPosition));
    }

    if (InstanceData->bShouldStopAllMontages)
    {
        Parts.Add(TEXT("StopAll"));
    }

    const FString Desc = FString::Join(Parts, TEXT(" | "));

	if (Formatting == EStateTreeNodeFormatting::RichText)
	{
		return FText::FromString(FString::Printf(TEXT("<b>%s</>"), *Desc));
	}
	return FText::FromString(Desc);
}
#endif

#undef LOCTEXT_NAMESPACE


