// Copyright - DMTesseracT

#include "Script/DmtAiScriptV2Common.h"
#include "Interface/DmtAiInterfaceV2.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"

void UDmtAiScriptV2Common::OnHostileTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
	// Ignore if the target has specified gameplay tags 
	if (HasAnyIgnoreGameplayTags(Target)) return;

	UpdatePerceivedTrackedActorState(Target, Stimulus, CharacterData, HostileActorData);

	// Request a state assessment rather than directly changing state
	RequestStateAssessment();
}

void UDmtAiScriptV2Common::OnHostileTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
	UpdateLostTrackedActorState(Target, Stimulus, CharacterData, HostileActorData);

	// Request state assessment when we lose targets
	RequestStateAssessment();
}

AActor* UDmtAiScriptV2Common::GetCurrentTarget_Implementation()
{
	// Forcefully return no target if we aren't in attack state
	if (GetCurrentAIState() != ENPCAIState::ATTACK) return nullptr;

	// Trace LOS to current target, valid if in LOS
	if (ValidateCurrentTarget()) return CurrentTarget;

	// Pick a new target or nullptr if we can't
	return SelectAttackTarget();
}

FVector UDmtAiScriptV2Common::GetYoungestAlertLocation()
{
	if (!WorldContext) return FVector::ZeroVector;

	FVector YoungestData = FVector::ZeroVector;
	float YoungestTime = ActorTrackingTimeout + 1.f;
	float CurrentTime = WorldContext->GetTimeSeconds();

	for (auto& HostileData : HostileActorData)
	{
		float TimeSinceStimulus = CurrentTime - HostileData.Value.LastStimulusTime;
		if (TimeSinceStimulus < YoungestTime)
		{
			YoungestTime = TimeSinceStimulus;
			YoungestData = HostileData.Value.LastSenseLocation;
		}
	}
	return YoungestData;
}

FVector UDmtAiScriptV2Common::GetAlertTargetLocation_Implementation()
{
	return GetYoungestAlertLocation();
}

bool UDmtAiScriptV2Common::HasTargetInMap(TMap<AActor*, FAiTrackedActorData>& Map, const bool RequireLineOfSight)
{
	if (Map.IsEmpty()) return false;

	for (auto& Pair : Map)
	{
		if (Pair.Value.TargetActor.IsValid())
		{
			if (RequireLineOfSight)
			{
				if (!Pair.Value.bSighted) continue; // If LOS is required and not valid on this target then move on
			}
			return true;
		}
	}
	return false;
}

bool UDmtAiScriptV2Common::HasAnyAttackTargets()
{
	return HasTargetInMap(HostileActorData, true);
}

bool UDmtAiScriptV2Common::HasAnyAlertTargets()
{
	return HasTargetInMap(HostileActorData, false);
}

void UDmtAiScriptV2Common::OnPerceptionTargetUpdated_Implementation(AActor* Target, FAIStimulus Stimulus)
{
	// Don't add any new perception targets that don't qualify due to ignored gameplay tags
	if (Stimulus.WasSuccessfullySensed() && HasAnyIgnoreGameplayTags(Target)) return;

	Super::OnPerceptionTargetUpdated_Implementation(Target, Stimulus);
}

AActor* UDmtAiScriptV2Common::SelectAttackTarget_Implementation()
{
	float HighestThreat = -1.f;
	AActor* ThreatActor = nullptr;

	for (auto& Pair : HostileActorData)
	{
		if (!Pair.Value.TargetActor.IsValid()) continue;

		// Only consider targets we can actually see
		if (Pair.Value.bSighted && Pair.Value.ThreatValue > HighestThreat)
		{
			ThreatActor = Pair.Value.TargetActor.Get();
			HighestThreat = Pair.Value.ThreatValue;
		}
	}

	return ThreatActor;
}

bool UDmtAiScriptV2Common::IsTargetInLOS(AActor* Target)
{
	if (!WorldContext || !Target || !ParentActor.Get()) return false;

	FHitResult Result;
	UKismetSystemLibrary::LineTraceSingle(
		WorldContext,
		ParentActor.Get()->GetActorLocation(),
		Target->GetActorLocation(),
		UEngineTypes::ConvertToTraceType(TargetTraceCollisionChannel),
		false,
		TArray<AActor*>{ParentActor.Get()},
		bDrawDebugTrace,
		Result,
		true
	);

	return Result.bBlockingHit && Result.GetActor() == Target;
}

bool UDmtAiScriptV2Common::ValidateCurrentTarget()
{
	if (CurrentTarget == nullptr) return false;

	// Check if target is in our hostile map and still sighted
	if (HostileActorData.Contains(CurrentTarget))
	{
		FAiTrackedActorData* TrackedData = HostileActorData.Find(CurrentTarget);
		if (TrackedData && TrackedData->bSighted)
		{
			return true;
		}
	}

	return false;
}

void UDmtAiScriptV2Common::UpdatePerceivedTrackedActorState(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData, TMap<AActor*, FAiTrackedActorData>& PerceptionMap)
{
	if (!Target || !WorldContext) return;

	FAiTrackedActorData* ActiveData;
	if (PerceptionMap.Contains(Target))
	{
		ActiveData = PerceptionMap.Find(Target);
	}
	else
	{
		ActiveData = &PerceptionMap.Add(Target, FAiTrackedActorData(Target, CharacterData.Name, 0.f));
	}

	ActiveData->LastStimulusTime = WorldContext->GetTimeSeconds();

	// Use proper stimulus type checking instead of string comparison
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		// Use the perception system's actual sight result
		ActiveData->bSighted = Stimulus.WasSuccessfullySensed();
		if (Stimulus.WasSuccessfullySensed())
		{
			ActiveData->LastSenseLocation = Stimulus.StimulusLocation;
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		ActiveData->LastSenseLocation = Stimulus.StimulusLocation;
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		ActiveData->LastSenseLocation = Stimulus.StimulusLocation;
	}
}

void UDmtAiScriptV2Common::UpdateLostTrackedActorState(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData, TMap<AActor*, FAiTrackedActorData>& PerceptionMap)
{
	if (!Target || !WorldContext) return;

	FAiTrackedActorData* ActiveData = PerceptionMap.Find(Target);

	if (!ActiveData) return;

	ActiveData->LastStimulusTime = WorldContext->GetTimeSeconds();

	// Use proper stimulus type checking
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		// Mark sight as lost when we lose the sight stimulus
		ActiveData->bSighted = false;
		ActiveData->LastSenseLocation = Stimulus.StimulusLocation;
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		ActiveData->LastSenseLocation = Stimulus.StimulusLocation;
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		ActiveData->LastSenseLocation = Stimulus.StimulusLocation;
	}
}

void UDmtAiScriptV2Common::RequestStateAssessment()
{
	if (!WorldContext) return;

	// Debounce state assessments to prevent thrashing
	float CurrentTime = WorldContext->GetTimeSeconds();
	if (CurrentTime - LastStateAssessmentTime < StateAssessmentCooldown)
	{
		return;
	}

	LastStateAssessmentTime = CurrentTime;

	// Clean up maps and assess state
	UpdateDataMaps();
}

void UDmtAiScriptV2Common::AssessAIState()
{
	// If AI is disabled by gameplay tags then don't consider states
	if (AiIsDisabled())
	{
		return;
	}

	ENPCAIState NewState = GetCurrentAIState();

	// Determine what state we SHOULD be in based on current conditions
	// Priority order: Attack > Alert > Patrol
	if (HasVisibleTarget(true)) // Use perception-based sight check
	{
		NewState = ENPCAIState::ATTACK;
	}
	else if (!HostileActorData.IsEmpty())
	{
		// We have hostile data but no visible targets - investigate
		NewState = ENPCAIState::ALERT;
	}
	else
	{
		// No hostiles at all - back to patrol
		NewState = ENPCAIState::PATROL;
	}

	// Only transition if state actually changed
	if (NewState != GetCurrentAIState())
	{
		SetCurrentAIState(NewState);

		if (ParentComponent.IsValid())
		{
			switch (NewState)
			{
			case ENPCAIState::ATTACK:
				ParentComponent->SetSightPeripheralVisionHalfAngle(AttackStateSightVisionAngle);
				break;

			case ENPCAIState::ALERT:
				ParentComponent->SetSightPeripheralVisionHalfAngle(AlertStateSightVisionAngle);
				break;

			case ENPCAIState::PATROL:
				ParentComponent->SetSightPeripheralVisionHalfAngle(PatrolStateSightVisionAngle);
				break;
			}
		}

		switch (NewState)
		{
		case ENPCAIState::ATTACK:
			OnTransitionToAttack();
			break;

		case ENPCAIState::ALERT:
			OnTransitionToAlert();
			break;

		case ENPCAIState::PATROL:
			OnTransitionToPatrol();
			break;
		}
	}
}

bool UDmtAiScriptV2Common::HasVisibleTarget(const bool bUsePerceptionSight)
{
	for (auto& HostileData : HostileActorData)
	{
		if (!HostileData.Value.TargetActor.IsValid()) continue;

		if (bUsePerceptionSight)
		{
			// Use the perception system's sight flag
			if (HostileData.Value.bSighted)
			{
				return true;
			}
		}
		else
		{
			// Manual LOS trace
			if (IsTargetInLOS(HostileData.Value.TargetActor.Get()))
			{
				return true;
			}
		}
	}
	return false;
}

void UDmtAiScriptV2Common::UpdateDataMaps()
{
	RemoveTimedOutTargetsFromMap(HostileActorData);
	RemoveTimedOutTargetsFromMap(NeutralActorData);
	RemoveTimedOutTargetsFromMap(FriendlyActorData);

	// Assess state after cleaning up the maps
	AssessAIState();
}

bool UDmtAiScriptV2Common::HasAnyIgnoreGameplayTags(AActor* Target)
{
	if (!Target || !Target->Implements<UDmtAiInterfaceV2>())
	{
		return false;
	}

	FGameplayTagContainer Container = IDmtAiInterfaceV2::Execute_GetActorGameplayTags(Target);
	return Container.HasAny(IgnoreTargetTrackingTags);
}

void UDmtAiScriptV2Common::RemoveTimedOutTargetsFromMap(TMap<AActor*, FAiTrackedActorData>& Map)
{
	if (!WorldContext) return;

	float CurrentTime = WorldContext->GetTimeSeconds();
	TArray<AActor*> Timeouts;

	for (auto& Pair : Map)
	{
		// Flag if the target is invalid
		if (!Pair.Value.TargetActor.IsValid())
		{
			Timeouts.Add(Pair.Key);
			continue;
		}

		// Check if the target has ignore tags and mark as timeout if so
		if (HasAnyIgnoreGameplayTags(Pair.Value.TargetActor.Get()))
		{
			Timeouts.Add(Pair.Key);
			continue;
		}

		// If we can still see them (perception-based), don't remove them
		if (Pair.Value.bSighted) continue;

		// If last stimulus was outside of timeout then flag for remove
		if (CurrentTime - Pair.Value.LastStimulusTime >= ActorTrackingTimeout)
		{
			Timeouts.Add(Pair.Key);
		}
	}

	// Remove all timed-out actors
	for (auto& Timeout : Timeouts)
	{
		Map.Remove(Timeout);
	}
}