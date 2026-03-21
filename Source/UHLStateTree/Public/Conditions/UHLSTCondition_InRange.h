// Pavel Penkov 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "UHLSTCondition_InRange.generated.h"

USTRUCT()
struct UHLSTATETREE_API FUHLSTCondition_InRangeInstanceData
{
	GENERATED_BODY()

	/** Origin actor for the distance test (uses actor world location). Any AActor is accepted. */
	UPROPERTY(EditAnywhere, Category = "Context", meta = (DisplayName = "Self Actor"))
	TObjectPtr<AActor> Character = nullptr;

	/** Optional second actor. When valid, its world location is used instead of Location. Any AActor is accepted. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (DisplayName = "Other Actor"))
	TObjectPtr<AActor> OtherCharacter = nullptr;

	/** World-space point used when Other Actor is null or invalid. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FVector Location = FVector::ZeroVector;

	/** Distance range in centimeters. Open/closed bounds follow FFloatRange rules. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FFloatRange Range = FFloatRange(0.0f, 1000.0f);

	/**
	 * When true and Self resolves to an ACharacter, subtracts that character's scaled capsule radius
	 * from the measured distance before the range test. Ignored for non-character actors.
	 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bIncludeSelfCapsuleRadius = true;

	/**
	 * When true, Other Actor resolves to an ACharacter, and Other Actor is used as the target,
	 * subtracts that character's scaled capsule radius from the measured distance. Ignored for non-character actors or when Location is used.
	 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bIncludeTargetCapsuleRadius = true;

	/** If true, the condition result is negated after the range test. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInverse = false;

	/** Draws debug primitives for the evaluated segment and range rings. */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebug = false;

	/** Lifetime of debug draws in seconds; used only when bDebug is true. */
	UPROPERTY(EditAnywhere, Category = "Debug", meta=(ClampMin="0.0", EditCondition="bDebug"))
	float DebugDuration = 2.5f;

	/** Optional note shown in the asset; does not affect runtime behavior. */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta=(MultiLine=true))
	FString Comment;

	/** Last Self actor pointer used to build CachedSelfCharacter. Not serialized. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> CachedSelfActorKey = nullptr;

	/** Cast cache for Self when it is an ACharacter. Not serialized. */
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CachedSelfCharacter = nullptr;

	/** Last Other actor pointer used to build CachedOtherCharacter. Not serialized. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> CachedOtherActorKey = nullptr;

	/** Cast cache for Other when it is an ACharacter. Not serialized. */
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CachedOtherCharacter = nullptr;
};

/**
 * Evaluates whether the distance from Self Actor to Other Actor (or Location) falls inside Range.
 * Capsule radius adjustments apply only when the corresponding actor is an ACharacter and the matching flag is set.
 */
USTRUCT(meta = (DisplayName="In Range", Category = "UHLStateTree"))
struct UHLSTATETREE_API FUHLSTCondition_InRange : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FUHLSTCondition_InRangeInstanceData;

	FUHLSTCondition_InRange() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FUHLSTCondition_InRangeInstanceData::StaticStruct(); }

	/** Computes range using world locations; refreshes transient ACharacter caches when actor pointers change. */
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	/** Short editor summary: actor target vs. Location fallback, plus formatted range in meters. */
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FName GetIconName() const override { return FName("StateTreeEditorStyle|Node.Movement"); }
	virtual FColor GetIconColor() const override { return UE::StateTree::Colors::DarkGreen; }
#endif
};


