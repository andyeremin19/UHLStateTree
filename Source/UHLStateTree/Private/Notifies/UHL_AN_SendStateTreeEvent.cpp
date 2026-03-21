// Pavel Penkov 2025 All Rights Reserved.

#include "Notifies/UHL_AN_SendStateTreeEvent.h"

#include "UHLStateTree.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UHL_AN_SendStateTreeEvent)

#if WITH_EDITOR
FLinearColor UUHL_AN_SendStateTreeEvent::GetEditorColor()
{
	return FLinearColor(FColor(46, 139, 87));
}
#endif

FString UUHL_AN_SendStateTreeEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		return FString::Printf(TEXT("UHL State Tree · %s"), *EventTag.ToString());
	}
	return FString(TEXT("UHL State Tree · (invalid tag)"));
}

UStateTreeComponent* UUHL_AN_SendStateTreeEvent::ResolveStateTreeComponent(AActor* const MeshOwner) const
{
	if (!IsValid(MeshOwner))
	{
		return nullptr;
	}

	const auto FindOnActor = [](AActor* const Actor) -> UStateTreeComponent*
	{
		return IsValid(Actor) ? Actor->FindComponentByClass<UStateTreeComponent>() : nullptr;
	};

	switch (Resolve)
	{
	case EUHLStateTreeNotifyResolve::OwnerActorOnly:
		return FindOnActor(MeshOwner);

	case EUHLStateTreeNotifyResolve::ControllerOnly:
	case EUHLStateTreeNotifyResolve::ControllerThenOwner:
	default:
		break;
	}

	if (APawn* const Pawn = Cast<APawn>(MeshOwner))
	{
		if (AController* const Controller = Pawn->GetController())
		{
			if (UStateTreeComponent* OnController = FindOnActor(Controller))
			{
				return OnController;
			}
		}
	}

	if (Resolve == EUHLStateTreeNotifyResolve::ControllerThenOwner)
	{
		return FindOnActor(MeshOwner);
	}

	return nullptr;
}

void UUHL_AN_SendStateTreeEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!EventTag.IsValid())
	{
		if (bLogFailures)
		{
			UE_LOG(LogUHLStateTree, Warning, TEXT("UUHL_AN_SendStateTreeEvent: EventTag is not set (%s)"),
				Animation ? *Animation->GetName() : TEXT("nullptr"));
		}
		return;
	}

	if (!IsValid(MeshComp))
	{
		if (bLogFailures)
		{
			UE_LOG(LogUHLStateTree, Warning, TEXT("UUHL_AN_SendStateTreeEvent: MeshComp is null (%s)"), *EventTag.ToString());
		}
		return;
	}

	AActor* const MeshOwner = MeshComp->GetOwner();
	if (!IsValid(MeshOwner))
	{
		if (bLogFailures)
		{
			UE_LOG(LogUHLStateTree, Warning, TEXT("UUHL_AN_SendStateTreeEvent: mesh has no owner (%s)"), *EventTag.ToString());
		}
		return;
	}

	UStateTreeComponent* const StateTreeComp = ResolveStateTreeComponent(MeshOwner);
	if (!IsValid(StateTreeComp))
	{
		if (bLogFailures)
		{
			UE_LOG(LogUHLStateTree, Warning,
				TEXT("UUHL_AN_SendStateTreeEvent: no UStateTreeComponent (tag=%s, owner=%s, resolve=%d)"),
				*EventTag.ToString(),
				*MeshOwner->GetName(),
				static_cast<int32>(Resolve));
		}
		return;
	}

	StateTreeComp->SendStateTreeEvent(EventTag, Payload);
}
