// Pavel Penkov 2025 All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "StateTreeEvaluatorBase.h"
#include "UHLSTEvaluator_OwnerASCTags.generated.h"

class AActor;

/**
 * Instance data for Owner ASC tags evaluator.
 */
USTRUCT()
struct UHLSTATETREE_API FUHLSTEvaluator_OwnerASCTagsInstanceData
{
	GENERATED_BODY()

	/**
	 * Actor whose AbilitySystemComponent is read (typically bind to the schema context "Actor", e.g. controlled pawn).
	 * If unset at runtime, the evaluator falls back to the context entry named "Actor".
	 */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Owner = nullptr;

	/**
	 * Weak cache of the ASC resolved from Owner in TreeStart; refreshed in Tick if it becomes invalid.
	 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC = nullptr;

	/**
	 * Owned gameplay tags copied from the cached ASC each Tick (empty if no valid ASC).
	 */
	UPROPERTY(EditAnywhere, Category = Output)
	FGameplayTagContainer Tags;
};

/**
 * Exposes the owner's ASC gameplay tags to the State Tree.
 * Resolves ASC and updates Tags on TreeStart and Tick.
 */
USTRUCT(meta = (DisplayName = "Owner ASC Tags", Category = "UHLStateTree"))
struct UHLSTATETREE_API FUHLSTEvaluator_OwnerASCTags : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FUHLSTEvaluator_OwnerASCTagsInstanceData;

	FUHLSTEvaluator_OwnerASCTags() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FName GetIconName() const override { return FName("StateTreeEditorStyle|Node.Time"); }
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::DarkBlue; }
#endif
};
