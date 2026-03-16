// Copyright - DMTesseracT

#pragma once

#include "CoreMinimal.h"
#include "Script/DmtAiV2ScriptBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DmtAiScriptV2Simple.generated.h"

/**
 * 
 */
UCLASS()
class DMT_AI_V2_API UDmtAiScriptV2Simple : public UDmtAiV2ScriptBase
{
	GENERATED_BODY()
	virtual void OnHostileTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData) override;
	virtual void OnHostileTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData) override;

public:

	virtual AActor* GetCurrentTarget_Implementation()override;

	UFUNCTION(BlueprintCallable, Category = "Alert")
	FVector GetYoungestAlertLocation();

	/*Select & Return a MoveTo location when in alert state */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Alert")
	FVector GetAlertTargetLocation();
	virtual FVector GetAlertTargetLocation_Implementation();

	// check if any hostile actors are available. Must have line of sight.
	UFUNCTION(BlueprintCallable, Category = "State")
	bool HasAnyAttackTargets() { return SelectAttackTarget() != nullptr; };

	// Check if any hostile actors exist, regardless of line of sight.
	UFUNCTION(BlueprintCallable, Category = "State")
	bool HasAnyAlertTargets() { return !HostileActorData.IsEmpty(); };

protected:

	virtual void OnPerceptionTargetUpdated_Implementation(AActor* Target, FAIStimulus Stimulus) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float AttackStateSightVisionAngle{180.f};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float AlertStateSightVisionAngle{ 60.f };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float PatrolStateSightVisionAngle{ 30.f };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TEnumAsByte<ECollisionChannel> TargetTraceCollisionChannel{ ECollisionChannel::ECC_Visibility };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	TEnumAsByte<EDrawDebugTrace::Type> bDrawDebugTrace;

	// Internal function for intermittently cleaning up tracked actors.
	UFUNCTION(BlueprintCallable)
	virtual void UpdateDataMaps();

	UFUNCTION()
	bool HasAnyIgnoreGameplayTags(AActor* Target);

	//Any perceived targets with any of these active gameplay tags will be ignored.If they are already being tracked, they will be forgotten on the next UpdateDataMaps call.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayTags")
	FGameplayTagContainer IgnoreTargetTrackingTags;

	// total number of game time seconds since last stim update before dropping a perceived actors from trackers.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Settings")
	float ActorTrackingTimeout{ 15.0f };

	virtual AActor* SelectAttackTarget_Implementation() override;


	virtual bool ValidateCurrentTarget();

	virtual void UpdatePerceivedTrackedActorState(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData, TMap<AActor*, FAiTrackedActorData>& PerceptionMap);
	virtual void UpdateLostTrackedActorState(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData, TMap<AActor*, FAiTrackedActorData>& PerceptionMap);

	bool HasVisibleTarget(const bool bUsePerceptionSight = false);

	UFUNCTION(BlueprintCallable, Category="AI State")
	void AssessAIState();

private:

	void RemoveTimedOutTargetsFromMap(TMap<AActor*, FAiTrackedActorData>& Map);
};

