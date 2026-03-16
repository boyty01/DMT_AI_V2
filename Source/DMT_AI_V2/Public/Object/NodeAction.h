// Copyright DMTesseract Ltd. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
#include "NodeAction.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class DMT_AI_V2_API UNodeAction : public UActorComponent
{
	GENERATED_BODY()

public:

    UNodeAction() {};

    /** Return true if this action is valid for the pawn right now */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Node Action")
    bool CanExecute(APawn* Pawn) const;
    virtual bool CanExecute_Implementation(APawn* Pawn) const { return true; }

    /** Execute the action asynchronously. Call OnFinished when done */
    virtual void ExecuteActionAsync(APawn* Pawn, TFunction<void()> OnFinished);

    /** Called by Blueprint when the async action is complete */
    UFUNCTION(BlueprintCallable, Category = "NodeAction")
    void OnFinished();

protected:

    TFunction<void()> OnFinishedCallback;
    
    /** Blueprint override for the actual action logic */
    UFUNCTION(BlueprintNativeEvent, Category = "NodeAction")
    void BP_ExecuteAction(APawn* Pawn);
    virtual void BP_ExecuteAction_Implementation(APawn* Pawn) {}

};