// Copyright - DMTesseracT


#include "ActorComponent/DmtAiBehaviorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Script/DmtAiV2ScriptBase.h"
#include "Interface/DmtAiInterfaceV2.h"
#include <Perception/AISense_Sight.h>
#include <Perception/AISenseConfig_Sight.h>


DEFINE_LOG_CATEGORY(DmtAiBehaviorComponent);

// Sets default values for this component's properties
UDmtAiBehaviorComponent::UDmtAiBehaviorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...
}

void UDmtAiBehaviorComponent::StopScript(FString Reason)
{
	if (!ActiveScript) return;

	ActiveScript->StopScript();
	DestroyScript();
}


void UDmtAiBehaviorComponent::DestroyScript()
{
	if (!ActiveScript) return;

	ActiveScript->ConditionalBeginDestroy();
	ActiveScript = nullptr;	
}

void UDmtAiBehaviorComponent::SetSightPeripheralVisionHalfAngle(const float NewAngle)
{
	if(PerceptionComponent.IsValid())
	{
		UAISenseConfig_Sight* SightConfig = Cast<UAISenseConfig_Sight>(PerceptionComponent->GetSenseConfig<UAISenseConfig_Sight>());
		if (SightConfig)
		{
			SightConfig->PeripheralVisionAngleDegrees = NewAngle;
			PerceptionComponent->RequestStimuliListenerUpdate(); // Apply changes
		}
	}
}

// Called when the game starts
void UDmtAiBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	Init();
}


void UDmtAiBehaviorComponent::OnPerceptionTargetUpdate(AActor* Actor, FAIStimulus Stimulus)
{
	OnPerceptionTargetUpdated.Broadcast(Actor, Stimulus);
	if (ActiveScript.Get()->IsValidLowLevelFast())
	{
		ActiveScript->OnPerceptionTargetUpdated(Actor, Stimulus);
	}
}

void UDmtAiBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (ActiveScript)
	{
		ActiveScript->ScriptTick(DeltaTime);
	}
}



void UDmtAiBehaviorComponent::Init()
{
	PerceptionComponent = GetOwnerPerceptionComponent();
	if (PerceptionComponent.IsValid())
	{
	/*
		TSubclassOf<UDmtAiV2ScriptBase> Script = ScriptClass.LoadSynchronous();
		if (!Script)
		{
			UE_LOG(DmtAiBehaviorComponent, Warning, TEXT("Failed to load Actor script for AI Component of owner: %s"), *GetOwner()->GetActorLabel());
			return;
		}
		*/
		if (!ScriptClass)
		{
			UE_LOG(LogClass, Warning, TEXT("Warning. No Script class assigned to AI Behavior component in Actor: %s"), *GetOwner()->GetFName().ToString());
			return;
		}

		ActiveScript = NewObject<UDmtAiV2ScriptBase>(this, ScriptClass);
		if (ActiveScript)
		{
			ActiveScript->Init(this);
			PerceptionComponent.Get()->OnTargetPerceptionUpdated.AddDynamic(this, &UDmtAiBehaviorComponent::OnPerceptionTargetUpdate);
			UE_LOG(DmtAiBehaviorComponent, Log, TEXT("Registered for Perception Updates"));
		}
		return;
	}
}

UAIPerceptionComponent* UDmtAiBehaviorComponent::GetOwnerPerceptionComponent()
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (AController* OwnerController = OwnerPawn->GetController())
		{
			return OwnerController->GetComponentByClass<UAIPerceptionComponent>();
		}
	}
	return nullptr;
}


