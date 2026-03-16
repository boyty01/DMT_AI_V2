// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DmtAiPatrolInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDmtAiPatrolInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DMT_AI_V2_API IDmtAiPatrolInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	//Invoked by a patrol node when an actor reaches it as part of its patrol.
	UFUNCTION(BlueprintImplementableEvent, Category = "AiPatrol")
	void ArrivedAtPatrolNode(class AAiPathNode* Node);
};
