// Pavel Penkov 2025 All Rights Reserved.

#include "Evaluators/UHLSTEvaluator_OwnerASCTags.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UHLSTEvaluator_OwnerASCTags)

#define LOCTEXT_NAMESPACE "UHLSTEvaluator_OwnerASCTags"

namespace UHLSTEvaluator_OwnerASCTags_Private
{
	static AActor* ResolveOwnerActor(
		const FUHLSTEvaluator_OwnerASCTagsInstanceData& InstanceData,
		FStateTreeExecutionContext& Context)
	{
		if (IsValid(InstanceData.Owner))
		{
			return InstanceData.Owner;
		}

		const FStateTreeDataView ActorView = Context.GetContextDataByName(FName(TEXT("Actor")));
		if (ActorView.IsValid())
		{
			return ActorView.GetMutablePtr<AActor>();
		}

		return nullptr;
	}

	static void RefreshCachedASC(
		FUHLSTEvaluator_OwnerASCTagsInstanceData& InstanceData,
		FStateTreeExecutionContext& Context)
	{
		AActor* const OwnerActor = ResolveOwnerActor(InstanceData, Context);
		if (!IsValid(OwnerActor))
		{
			InstanceData.CachedASC = nullptr;
			return;
		}

		if (UAbilitySystemComponent* const ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor))
		{
			InstanceData.CachedASC = ASC;
		}
		else
		{
			InstanceData.CachedASC = nullptr;
		}
	}

	static void RefreshTagsOutput(
		FUHLSTEvaluator_OwnerASCTagsInstanceData& InstanceData,
		FStateTreeExecutionContext& Context)
	{
		UAbilitySystemComponent* ASC = InstanceData.CachedASC.Get();
		if (!IsValid(ASC))
		{
			RefreshCachedASC(InstanceData, Context);
			ASC = InstanceData.CachedASC.Get();
		}

		if (IsValid(ASC))
		{
			InstanceData.Tags = ASC->GetOwnedGameplayTags();
		}
		else
		{
			InstanceData.Tags.Reset();
		}
	}
}

void FUHLSTEvaluator_OwnerASCTags::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.CachedASC = nullptr;
	InstanceData.Tags.Reset();
	UHLSTEvaluator_OwnerASCTags_Private::RefreshTagsOutput(InstanceData, Context);
}

void FUHLSTEvaluator_OwnerASCTags::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.CachedASC = nullptr;
	InstanceData.Tags.Reset();
}

void FUHLSTEvaluator_OwnerASCTags::Tick(FStateTreeExecutionContext& Context, const float /*DeltaTime*/) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UHLSTEvaluator_OwnerASCTags_Private::RefreshTagsOutput(InstanceData, Context);
}

#if WITH_EDITOR
FText FUHLSTEvaluator_OwnerASCTags::GetDescription(
	const FGuid&,
	FStateTreeDataView,
	const IStateTreeBindingLookup&,
	const EStateTreeNodeFormatting) const
{
	return LOCTEXT("EvaluatorDescription", "Owner ASC tags → Tags");
}
#endif

#undef LOCTEXT_NAMESPACE
