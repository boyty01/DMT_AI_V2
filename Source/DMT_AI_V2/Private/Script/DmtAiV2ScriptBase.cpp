// Copyright - DMTesseracT


#include "Script/DmtAiV2ScriptBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "Interface/DmtAiInterfaceV2.h"
#include "AIController.h"
#include "ActorComponent/DmtAiBehaviorComponent.h"

void UDmtAiV2ScriptBase::Init(UDmtAiBehaviorComponent* OwnerBehaviorComponent)
{
	if (!OwnerBehaviorComponent) return;

	OwnerBehaviorComponent->OnPerceptionTargetUpdated.AddDynamic(this, &UDmtAiV2ScriptBase::OnPerceptionTargetUpdated);
	WorldContext = OwnerBehaviorComponent->GetWorld();
	ParentComponent = OwnerBehaviorComponent;
	ParentActor = OwnerBehaviorComponent->GetOwner();

	BP_Init(OwnerBehaviorComponent);
	return;

}

void UDmtAiV2ScriptBase::OnPerceptionTargetUpdated_Implementation(AActor* Target, FAIStimulus Stimulus)
{


	if (Target->Implements<UDmtAiInterfaceV2>())
	{
		FCharacterAiData CData = IDmtAiInterfaceV2::Execute_GetCharacterAiData(Target);
		
		if (Stimulus.WasSuccessfullySensed())
		{
			HostileFactions.HasTag(CData.Faction) ? OnHostileTargetPerceived(Target, Stimulus, CData)
				: FriendlyFactions.HasTag(CData.Faction) ? OnFriendlyTargetPerceived(Target, Stimulus, CData)
				: OnNeutralTargetPerceived(Target, Stimulus, CData);
			return;
		}

		HostileFactions.HasTag(CData.Faction) ? OnHostileTargetLost(Target, Stimulus, CData)
			: FriendlyFactions.HasTag(CData.Faction) ? OnFriendlyTargetLost(Target, Stimulus, CData)
			: OnNeutralTargetLost(Target, Stimulus, CData);
		
		return;
	}
	return;
}


bool UDmtAiV2ScriptBase::AiIsDisabled()
{
	if (!ParentActor.Get() || !ParentActor->Implements<UDmtAiInterfaceV2>())
	{
		return false;
	}

	FGameplayTagContainer ActiveTags = IDmtAiInterfaceV2::Execute_GetActorGameplayTags(ParentActor.Get());
	
	return ActiveTags.HasAny(AiDisabledTags);
}

AAIController* UDmtAiV2ScriptBase::GetAiController()
{
	if (!ParentActor.Get()) return nullptr;

	APawn* AsPawn = Cast<APawn>(ParentActor.Get());

	if (AsPawn)
	{
		return Cast<AAIController>(AsPawn->GetController());
	}

	return nullptr;
}

void UDmtAiV2ScriptBase::AddActorToMap(AActor* Actor, FCharacterAiData Data, TMap<AActor*, FAiTrackedActorData>& Map)
{
	FAiTrackedActorData NewData(Actor, Data.Name, 0.f);
	Map.Add(Actor, NewData);
}

void UDmtAiV2ScriptBase::OnHostileTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
}

void UDmtAiV2ScriptBase::OnNeutralTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
}

void UDmtAiV2ScriptBase::OnFriendlyTargetPerceived_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
}

void UDmtAiV2ScriptBase::OnHostileTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
}

void UDmtAiV2ScriptBase::OnNeutralTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
}

void UDmtAiV2ScriptBase::OnFriendlyTargetLost_Implementation(AActor* Target, FAIStimulus Stimulus, FCharacterAiData CharacterData)
{
}
