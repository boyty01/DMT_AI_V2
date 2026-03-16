// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AITypes.h"
#include "AiPathFollowComponent.generated.h"


class AAiPathNode;
class AAIController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DMT_AI_V2_API UAiPathFollowComponent : public UActorComponent
{
	GENERATED_BODY()

    UAiPathFollowComponent();

public:
    /** Start following a route of nodes */
    UFUNCTION(BlueprintCallable, Category = "NodeFollower")
    void FollowNodeRoute(const TArray<AAiPathNode*>& Route);

    /** Stop following the current route */
    UFUNCTION(BlueprintCallable, Category = "NodeFollower")
    void StopFollowingRoute();

    void OnMoveRequestFinished(
        FAIRequestID RequestID,
        const FPathFollowingResult& Result);
protected:
    /** Moves to the next node in the sequence */
    void MoveToNextNode();

    /** Called when a node completes all its actions */
    void OnNodeCompleted();

    /** The current node route */
    TArray<AAiPathNode*> NodeRoute;

    /** Index of the next node to visit */
    int32 CurrentNodeIndex;

    /** Cached pawn reference */
    APawn* OwnerPawn;

    /** Cached AIController reference */
    AAIController* OwnerController;

    /** Whether the component is actively following a route */
    bool bIsFollowing;

};
