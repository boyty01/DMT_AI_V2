// Copyright - DMTesseracT


#include "Script/DmtAiScriptV2Simple.h"
#include "Interface/DmtAiInterfaceV2.h"

void UDmtAiScriptV2Simple::OnHostileTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
	if (HasAnyIgnoreGameplayTags(Target)) return;


	UpdatePerceivedTrackedActorState(Target, Stimulus, CharacterData, HostileActorData);

}

void UDmtAiScriptV2Simple::OnHostileTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
	UpdateLostTrackedActorState(Target, Stimulus, CharacterData, HostileActorData);
}

AActor* UDmtAiScriptV2Simple::GetCurrentTarget_Implementation()
{
//pick a new target or nullptr if we can't
	return SelectAttackTarget();
}

FVector UDmtAiScriptV2Simple::GetYoungestAlertLocation()
{
	FVector YoungestData = FVector();
	float YoungestTime = ActorTrackingTimeout + 1;
	for (auto& HostileData : HostileActorData)
	{
		if (HostileData.Value.LastStimulusTime > WorldContext->GetTimeSeconds() - YoungestTime)
		{
			YoungestTime = HostileData.Value.LastStimulusTime;
			YoungestData = HostileData.Value.LastSenseLocation;
		}
	}
	return YoungestData;
}

FVector UDmtAiScriptV2Simple::GetAlertTargetLocation_Implementation()
{
	return FVector();
}

AActor* UDmtAiScriptV2Simple::SelectAttackTarget_Implementation()
{
	float HighestThreat = -1.f;
	AActor* ThreatActor = nullptr;
	for (auto& Pair : HostileActorData)
	{
		if (Pair.Value.ThreatValue > HighestThreat && Pair.Value.bSighted)
		{
			ThreatActor = Pair.Value.TargetActor.Get();
			HighestThreat = Pair.Value.ThreatValue;
		}
	}
	return ThreatActor;
}

bool UDmtAiScriptV2Simple::ValidateCurrentTarget()
{
	return SelectAttackTarget() != nullptr;
}

void UDmtAiScriptV2Simple::UpdatePerceivedTrackedActorState(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData, TMap<AActor*, FAiTrackedActorData>& PerceptionMap)
{
	FAiTrackedActorData* ActiveData;
	if (PerceptionMap.Contains(Target))
	{
		FAiTrackedActorData& ExistingData = *PerceptionMap.Find(Target);
		ActiveData = &ExistingData;
		if (ActiveData->bSighted)
			return;
	}
	else
	{
		ActiveData = &PerceptionMap.Add(Target, FAiTrackedActorData(Target, CharacterData.Name, 0.f));
	}

	ActiveData->LastStimulusTime = WorldContext->GetTimeSeconds();
	ActiveData->bSighted = Stimulus.Type.Name.ToString().Contains("Sight");
	ActiveData->LastSenseLocation = Stimulus.StimulusLocation;

	UpdateDataMaps();
}

void UDmtAiScriptV2Simple::UpdateLostTrackedActorState(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData, TMap<AActor*, FAiTrackedActorData>& PerceptionMap)
{

	// We only care about Sight stim, other stim expiry shouldn't refresh any data.
	FAiTrackedActorData* ActiveData;
	if (PerceptionMap.Contains(Target) && Stimulus.Type.Name.ToString().Contains("Sight"))
	{
		FAiTrackedActorData& ExistingData = *PerceptionMap.Find(Target);
		ActiveData = &ExistingData;
		ActiveData->LastStimulusTime = WorldContext->GetTimeSeconds();		
		ActiveData->bSighted = false;
		ActiveData->LastSenseLocation = Stimulus.StimulusLocation;
	}
	
	UpdateDataMaps();
}

bool UDmtAiScriptV2Simple::HasVisibleTarget(const bool bUsePerceptionSight)
{
	return SelectAttackTarget() != nullptr;
}

void UDmtAiScriptV2Simple::AssessAIState()
{
	// if Ai is disabled by gameplay tags then don't consider states.
	if (AiIsDisabled())
	{
		return;
	}

	// Transition to patrol
	if (HostileActorData.IsEmpty() && GetCurrentAIState() != ENPCAIState::PATROL)
	{
		SetCurrentAIState(ENPCAIState::PATROL);
		ParentComponent->SetSightPeripheralVisionHalfAngle(PatrolStateSightVisionAngle);
		OnTransitionToPatrol();
	}

	if (!HostileActorData.IsEmpty() && !HasVisibleTarget() && GetCurrentAIState() != ENPCAIState::ALERT)
	{
		SetCurrentAIState(ENPCAIState::ALERT);
		ParentComponent->SetSightPeripheralVisionHalfAngle(AlertStateSightVisionAngle);
		OnTransitionToAlert();
	}

	if (!HostileActorData.IsEmpty() && HasVisibleTarget() && GetCurrentAIState() != ENPCAIState::ATTACK)
	{
		SetCurrentAIState(ENPCAIState::ATTACK);
		ParentComponent->SetSightPeripheralVisionHalfAngle(AttackStateSightVisionAngle);
		OnTransitionToAttack();
	}

}

void UDmtAiScriptV2Simple::OnPerceptionTargetUpdated_Implementation(AActor* Target, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed() && HasAnyIgnoreGameplayTags(Target)) return;
	Super::OnPerceptionTargetUpdated_Implementation(Target, Stimulus);
}

void UDmtAiScriptV2Simple::UpdateDataMaps()
{
	RemoveTimedOutTargetsFromMap(HostileActorData);
	RemoveTimedOutTargetsFromMap(NeutralActorData);
	RemoveTimedOutTargetsFromMap(FriendlyActorData);

	AssessAIState();
}

bool UDmtAiScriptV2Simple::HasAnyIgnoreGameplayTags(AActor* Target)
{
	if (Target->Implements<UDmtAiInterfaceV2>())
	{
		FGameplayTagContainer Container = IDmtAiInterfaceV2::Execute_GetActorGameplayTags(Target);
		return Container.HasAny(IgnoreTargetTrackingTags);
	}
	return false;
}

void UDmtAiScriptV2Simple::RemoveTimedOutTargetsFromMap(TMap<AActor*, FAiTrackedActorData>& Map)
{
	if (!WorldContext) return;
	float CurrentTime = WorldContext->TimeSeconds;
	TArray<AActor*> Timeouts;

	for (auto& Pair : Map)
	{
		// flag if the target is invalid.
		if (!Pair.Value.TargetActor.IsValid())
		{
			Timeouts.Add(Pair.Value.TargetActor.Get());
			continue;
		}

		// check if the target has ignore tags and mark as timeout if so.
		if (HasAnyIgnoreGameplayTags(Pair.Value.TargetActor.Get()))
		{
			Timeouts.Add(Pair.Value.TargetActor.Get());
			continue;
		}

		// if last stim was outside of timeout then flag for remove.
		if (CurrentTime - Pair.Value.LastStimulusTime >= ActorTrackingTimeout)
		{
			Timeouts.Add(Pair.Value.TargetActor.Get());
		}
	}

	for (auto& Timeout : Timeouts)
	{
		Map.Remove(Timeout);
	}
}