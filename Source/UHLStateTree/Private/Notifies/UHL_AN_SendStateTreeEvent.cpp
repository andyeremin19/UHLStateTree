// Copyright (c) 2024 NextGenium


#include "Notifies/UHL_AN_SendStateTreeEvent.h"

#include "Components/StateTreeComponent.h"

void UUHL_AN_SendStateTreeEvent::Notify(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	
	AActor* TargetActor = MeshComp->GetOwner();
	if (!TargetActor) return;

	if (APawn* Pawn =  Cast<APawn>(TargetActor))
	{
		AController* Controller = Pawn->GetController();
		if (!Controller) return;
		UStateTreeComponent* STComp = Controller->FindComponentByClass<UStateTreeComponent>();
		if (STComp!= nullptr)
		{
			STComp->SendStateTreeEvent(EventTag, Payload);
		}
	}
}
