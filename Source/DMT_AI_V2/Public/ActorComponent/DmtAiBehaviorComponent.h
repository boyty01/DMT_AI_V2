// Copyright - DMTesseracT

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "Perception/AIPerceptionTypes.h"
#include "DmtAiBehaviorComponent.generated.h"

USTRUCT(BlueprintType)
struct FCharacterAiData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPCData")
	FName Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPCData")
	FGameplayTag Faction;
};


DECLARE_LOG_CATEGORY_EXTERN(DmtAiBehaviorComponent, Log, Verbose);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerceptionTargetUpdated, AActor*, Actor, FAIStimulus, Stimulus);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DMT_AI_V2_API UDmtAiBehaviorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDmtAiBehaviorComponent();

	UFUNCTION(BlueprintCallable, Category ="NPC Script")
	void StopScript(FString Reason);

	UFUNCTION(BlueprintCallable, Category="NPC Script")
	class UDmtAiV2ScriptBase* GetActiveScript() { return ActiveScript; };

	//Delegate for when owners AI Perception component target data is updated. 
	UPROPERTY(BlueprintAssignable)
	FOnPerceptionTargetUpdated OnPerceptionTargetUpdated;

	UFUNCTION(BlueprintCallable, Category ="NPC Script")
	void DestroyScript();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category ="Debug")
	bool IsDebugTraceEnabled() { return bEnableDebugTrace; };

	UFUNCTION(BlueprintCallable, Category ="NPC Script")
	void SetSightPeripheralVisionHalfAngle(const float NewAngle);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TWeakObjectPtr<class UAIPerceptionComponent> PerceptionComponent;

	//The AI script class to run for this NPC.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category ="NPC Settings")
	TSubclassOf<UDmtAiV2ScriptBase> ScriptClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category ="Debug")
	bool bEnableDebugTrace;

	UFUNCTION()
	void OnPerceptionTargetUpdate(AActor* Actor, struct FAIStimulus Stimulus);


	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;

private:

	UPROPERTY()
	TObjectPtr<class UDmtAiV2ScriptBase> ActiveScript;

	void Init();

	UAIPerceptionComponent* GetOwnerPerceptionComponent();
		
};
