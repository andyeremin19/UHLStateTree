// Copyright (c) 2024 NextGenium

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UHL_AN_SendStateTreeEvent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Category="UnrealHelperLibrary")
class UHLSTATETREE_API UUHL_AN_SendStateTreeEvent : public UAnimNotify
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="SendStateTreeEvent")
	FGameplayTag EventTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SendStateTreeEvent")
	FInstancedStruct Payload;

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
