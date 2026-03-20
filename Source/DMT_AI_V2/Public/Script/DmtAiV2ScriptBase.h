// Copyright - DMTesseracT

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActorComponent/DmtAiBehaviorComponent.h"
#include "GameplayTags.h"
#include "Perception/AIPerceptionTypes.h"
#include "DmtAiV2ScriptBase.generated.h"

/**
 * 
 */


UENUM(BlueprintType)
enum class ENPCAIState : uint8
{
	PATROL,
	ALERT,
	ATTACK
};

UENUM(BlueprintType)
enum class FAiFactionFamily : uint8
{
	FRIENDLY,
	NEUTRAL,
	HOSTILE
};


USTRUCT(BlueprintType)
struct FAiTrackedActorData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category ="Actor Name")
	FName Name{ NAME_None };

	UPROPERTY(BlueprintReadWrite, Category="Actor")
	TWeakObjectPtr<AActor> TargetActor;

	// if true, the sight stim is still valid
	UPROPERTY(BlueprintReadWrite, Category="Perception")
	bool bSighted{ false };

	// the last world location that any sense stimuli came from.
	UPROPERTY(BlueprintReadWrite, Category="Perception")
	FVector LastSenseLocation;

	// game time of the last stim event for this actor data.
	UPROPERTY(BlueprintReadWrite, Category ="Perception")
	float LastStimulusTime;

	// Arbitrary number that can be used to select attack targets if multiple exist.
	UPROPERTY(BlueprintReadWrite, Category="Threat")
	float ThreatValue{0.f};

	
	FAiTrackedActorData() {};
	FAiTrackedActorData(AActor* Actor, const FName CharacterName, const float Threat)
	{
		TargetActor = Actor;
		Name = CharacterName;
		ThreatValue = Threat;
	}

	bool operator== (AActor* Comparator) const
	{
		return TargetActor == Comparator;
	}

	bool operator>(const float InThreat)const
	{
		return ThreatValue > InThreat;
	};

	void AddThreat(const float ValueToAdd)
	{
		ThreatValue += ValueToAdd;
	}

	void ResetThreat()
	{
		ThreatValue = 0;
	};

	void DeductThreat(const float ValueToDeduct)
	{
		ThreatValue -= FMath::Clamp(ValueToDeduct, 0, ThreatValue);
	}

};


struct FAIStimulus;
struct FCharacterAiData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionToAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionToPatrol);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionToAlert);

UCLASS(Blueprintable, BlueprintType)
class DMT_AI_V2_API UDmtAiV2ScriptBase : public UObject
{
	GENERATED_BODY()


public:

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnTransitionToAttack OnTransitionToAttackDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTransitionToPatrol OnTransitionToPatrolDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTransitionToAlert OnTransitionToAlertDelegate;

	UFUNCTION(BlueprintNativeEvent, Category = "Tick")
	void ScriptTick(const float Delta);
	virtual void ScriptTick_Implementation(const float Delta) {};

	virtual void Init(class UDmtAiBehaviorComponent* OwnerBehaviorComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "Init")
	void BP_Init(class UDmtAiBehaviorComponent* OwnerBehaviorComponent);
	void BP_Init_Implementation(class UDmtAiBehaviorComponent* OwnerBehaviorComponent) {};

	UFUNCTION(BlueprintNativeEvent, Category = "Perception")
	void OnPerceptionTargetUpdated(AActor* Target, FAIStimulus Stimulus);

	UFUNCTION(BlueprintNativeEvent, Category = "Hostile Targets")
	void OnHostileTargetPerceived(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);
	virtual void OnHostileTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);

	UFUNCTION(BlueprintNativeEvent, Category = "Neutral Targets")
	void OnNeutralTargetPerceived(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);
	virtual void OnNeutralTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);

	UFUNCTION(BlueprintNativeEvent, Category = "Friendly Targets")
	void OnFriendlyTargetPerceived(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);
	virtual void OnFriendlyTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);

	UFUNCTION(BlueprintNativeEvent, Category = "Hostile Targets")
	void OnHostileTargetLost(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);
	virtual void OnHostileTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);

	UFUNCTION(BlueprintNativeEvent, Category = "Neutral Targets")
	void OnNeutralTargetLost(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);
	virtual void OnNeutralTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);

	UFUNCTION(BlueprintNativeEvent, Category = "Friendly Targets")
	void OnFriendlyTargetLost(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);
	virtual void OnFriendlyTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData);

	//Handle selecting & returning the current attack target. 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	AActor* GetCurrentTarget();
	virtual AActor* GetCurrentTarget_Implementation() { return nullptr; };

	UFUNCTION(BlueprintCallable, Category = "Hostile Targets")
	TMap<AActor*, FAiTrackedActorData> GetActiveHostileTargets() { return HostileActorData; };

	UFUNCTION(BlueprintCallable, Category = "Neutral Targets")
	TMap<AActor*, FAiTrackedActorData> GetActiveNeutralTargets() { return NeutralActorData; };

	UFUNCTION(BlueprintCallable, Category = "Friendly Targets")
	TMap<AActor*, FAiTrackedActorData> GetFriendlyTargets() { return FriendlyActorData; };

	UFUNCTION(BlueprintCallable, Category = "AI State")
	ENPCAIState GetCurrentAIState() { return CurrentState; };

	UFUNCTION(BlueprintCallable, Category = "AI State")
	void SetCurrentAIState(const ENPCAIState NewState) { CurrentState = NewState; };

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void StopScript();

protected:

	//Checks if the AI is in a state to utilise perception. If its disabled, perception updates will not be processed.
	UFUNCTION(BlueprintCallable, Category = "AI State")
	bool AiIsDisabled();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Owners")
	class AAIController* GetAiController();

	UFUNCTION(BlueprintCallable, Category = "Owners")
	AActor* GetParentActor() { return ParentActor.Get(); };

	UFUNCTION(BlueprintCallable, Category ="Owners")
	UDmtAiBehaviorComponent* GetPerception() { return ParentComponent.Get(); };

	UFUNCTION(BlueprintCallable, Category ="Perceived Actors")
	void AddActorToMap(AActor* Actor, FCharacterAiData Data, TMap<AActor*, FAiTrackedActorData>& Map);

	UFUNCTION(BlueprintCallable, Category ="Combat")
	void SetCurrentTarget(AActor* NewTarget) { CurrentTarget = NewTarget; };

	//Factions that this npc should consider hostile
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC Settings")
	FGameplayTagContainer HostileFactions;

	// Factions that this npc should consider friendly
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC Settings")
	FGameplayTagContainer FriendlyFactions;

	// gameplay tags that if present will cause the script to ignore perception events.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC Settings")
	FGameplayTagContainer AiDisabledTags;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category ="Combat")
	AActor* SelectAttackTarget();
	virtual AActor* SelectAttackTarget_Implementation() { return nullptr; };

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
	void OnTransitionToPatrol();
	virtual void OnTransitionToPatrol_Implementation() {};

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
	void OnTransitionToAlert();
	virtual void OnTransitionToAlert_Implementation() {};

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
	void OnTransitionToAttack();
	virtual void OnTransitionToAttack_Implementation() {};


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Perceived Actors")
	TMap<AActor*, FAiTrackedActorData> HostileActorData;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Perceived Actors")
	TMap<AActor*, FAiTrackedActorData> NeutralActorData;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Perceived Actors")
	TMap<AActor*, FAiTrackedActorData> FriendlyActorData;

	UWorld* WorldContext;

	TWeakObjectPtr<UDmtAiBehaviorComponent> ParentComponent;

	TWeakObjectPtr<AActor> ParentActor;

	UPROPERTY()
	AActor* CurrentTarget;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category="World Context")
	UWorld* GetWorldContext() { return WorldContext; };

private:

	UPROPERTY()
	ENPCAIState CurrentState{ENPCAIState::PATROL};

	UPROPERTY()
	FVector LastHearingStimulusLocation;

	UPROPERTY()
	FVector LastSightStimulusLocation;




};
