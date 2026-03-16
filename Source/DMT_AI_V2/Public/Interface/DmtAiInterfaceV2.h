// Copyright - DMTesseracT

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DmtAiInterfaceV2.generated.h"



// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDmtAiInterfaceV2 : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DMT_AI_V2_API IDmtAiInterfaceV2
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.


public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category ="AI")
	FGameplayTag GetFaction();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category ="AI")
	struct  FCharacterAiData GetCharacterAiData();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category ="AI")
	FVector GetYoungestAlertLocation();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="AI")
	UObject* GetPatrolObject();

	//Animation event that uses a byte to allow for decoupled systems to implement their own switches. Used when the calling logic doesn't need to know when the montage has finished.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="AI")
	void AiAnimationEvent(uint8 Key);

	//Get animation montage and target mesh. Use in AI Behavior tree to manage flow control appropriately around montage length.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AI")
	bool GetAiAnimationFromEvent(uint8 Key, UAnimMontage*& Montage, USkeletalMeshComponent*& SkeletalMeshTarget);

	//Allows actors to provide gameplay tag containers that are relevant to any implementations without causing dependencies at source level. Base AI Scripts use this when querying any gameplay tags.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category ="AI")
	FGameplayTagContainer GetActorGameplayTags();

};
