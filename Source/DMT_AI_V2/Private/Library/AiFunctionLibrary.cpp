// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/AiFunctionLibrary.h"
#include "ActorComponent/DmtAiBehaviorComponent.h"
#include "Script/DmtAiV2ScriptBase.h"

bool UAiFunctionLibrary::GetActorAiScript(AActor* TargetActor, UDmtAiV2ScriptBase*& Script)
{
	Script = nullptr;
	UDmtAiBehaviorComponent* Behavior;
	if (GetDmtAiBehaviorComponent(TargetActor, Behavior))
	{
		Script = Behavior->GetActiveScript();
	}
	return Script != nullptr;
}

bool UAiFunctionLibrary::GetDmtAiBehaviorTarget(AActor* TargetActor, AActor*& BehaviorTarget)
{
	BehaviorTarget = nullptr;
	UDmtAiV2ScriptBase* Script;
	if (GetActorAiScript(TargetActor, Script))
	{
		BehaviorTarget = Script->GetCurrentTarget();
	}
	return BehaviorTarget != nullptr;
}

bool UAiFunctionLibrary::GetDmtAiBehaviorComponent(AActor* TargetActor, UDmtAiBehaviorComponent*& BehaviorComponent)
{
	if (!TargetActor) return false;

	BehaviorComponent = nullptr;
	BehaviorComponent = TargetActor->GetComponentByClass<UDmtAiBehaviorComponent>();

	return BehaviorComponent != nullptr;
}
