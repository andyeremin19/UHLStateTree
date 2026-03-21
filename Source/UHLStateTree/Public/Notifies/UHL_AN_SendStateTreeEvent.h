// Pavel Penkov 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UHL_AN_SendStateTreeEvent.generated.h"

class UStateTreeComponent;

/**
 * Where to look for a UStateTreeComponent when the notify fires on a skeletal mesh.
 * Default matches the common AI setup: State Tree (e.g. UStateTreeAIComponent / UUHLStateTreeAIComponent) on the pawn's controller.
 */
UENUM(BlueprintType)
enum class EUHLStateTreeNotifyResolve : uint8
{
	/** Pawn owner only: use GetController() and search on that actor. Fails if there is no controller or no component on it. */
	ControllerOnly UMETA(DisplayName = "Controller only"),

	/** Mesh GetOwner() only (any actor). Useful when the State Tree component lives on the same actor as the mesh. */
	OwnerActorOnly UMETA(DisplayName = "Mesh owner only"),

	/** Try ControllerOnly first, then OwnerActorOnly. */
	ControllerThenOwner UMETA(DisplayName = "Controller, then mesh owner"),
};

/**
 * Anim notify: sends a State Tree event (gameplay tag + optional payload) to a UStateTreeComponent resolved from the playing mesh.
 * Intended for montages on characters/pawns driven by an AI (or other) controller that owns the brain / State Tree component.
 */
UCLASS(Blueprintable, Category = "UHL State Tree", meta = (DisplayName = "UHL · Send State Tree Event"))
class UHLSTATETREE_API UUHL_AN_SendStateTreeEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	/** Event tag delivered to the State Tree; must be valid or the notify no-ops. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UHL State Tree|Event")
	FGameplayTag EventTag = FGameplayTag::EmptyTag;

	/** Optional payload struct instance; must match what the receiving tree / schema expects. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UHL State Tree|Event")
	FInstancedStruct Payload;

	/** How to find the UStateTreeComponent from the mesh owner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UHL State Tree|Event")
	EUHLStateTreeNotifyResolve Resolve = EUHLStateTreeNotifyResolve::ControllerOnly;

	/** If true, prints a LogUHLStateTree warning when the event is skipped (invalid tag, missing owner, or no component). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UHL State Tree|Debug")
	bool bLogFailures = false;

protected:
#if WITH_EDITOR
	/** Avoids firing State Tree side effects while scrubbing in the animation editor. */
	virtual bool ShouldFireInEditor() { return false; }

	virtual FLinearColor GetEditorColor() override;
#endif

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	UStateTreeComponent* ResolveStateTreeComponent(AActor* MeshOwner) const;
};
