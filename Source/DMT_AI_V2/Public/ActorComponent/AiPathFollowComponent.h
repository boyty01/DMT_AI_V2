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

    // whether a route is currently being followed. 
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="NodeFollower")
    bool IsFollowingRoute() { return bIsFollowing; };

    /** Start following a route of nodes */
    UFUNCTION(BlueprintCallable, Category = "NodeFollower")
    void FollowNodeRoute(const TArray<AAiPathNode*>& Route);

    /** Stop following the current route */
    UFUNCTION(BlueprintCallable, Category = "NodeFollower")
    void StopFollowingRoute();

    /** Pause movement — preserves route and current node index so ResumePathFollow picks up from the same node */
    UFUNCTION(BlueprintCallable, Category = "NodeFollower")
    void PausePathFollow();

    /** Resume movement from the node that was next when PausePathFollow was called */
    UFUNCTION(BlueprintCallable, Category = "NodeFollower")
    void ResumePathFollow();

    /** Returns true if the route is paused (not stopped, just paused) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NodeFollower")
    bool IsPathFollowPaused() const { return bIsPaused; }

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

    /** Number of times to retry a failed move before skipping the node */
    UPROPERTY(EditAnywhere, Category = "NodeFollower")
    int32 MaxMoveRetries = 3;

    /** Seconds to wait before retrying a failed move */
    UPROPERTY(EditAnywhere, Category = "NodeFollower")
    float RetryDelay = 1.0f;

    /** How often (seconds) to sample position and check for stuck-against-another-agent */
    UPROPERTY(EditAnywhere, Category = "NodeFollower|Avoidance")
    float StuckCheckInterval = 0.5f;

    /** Minimum distance (cm) the NPC must travel per StuckCheckInterval to not be considered stuck */
    UPROPERTY(EditAnywhere, Category = "NodeFollower|Avoidance")
    float StuckDistanceThreshold = 25.0f;

    /** Base yield duration (seconds) when stuck is detected. Randomised ±50% to break symmetry between NPCs */
    UPROPERTY(EditAnywhere, Category = "NodeFollower|Avoidance")
    float YieldDuration = 0.75f;

private:
    int32 CurrentRetryCount = 0;
    bool bIsYielding = false;
    bool bIsPaused = false;

    FTimerHandle RetryTimerHandle;
    FTimerHandle StuckCheckTimerHandle;
    FTimerHandle YieldTimerHandle;

    FVector LastStuckCheckLocation = FVector::ZeroVector;

    void RetryCurrentNode();

    /** Repeating check: if we haven't moved far enough, yield to let other agents pass */
    void CheckIfStuck();

    /** Stops movement and waits a randomised duration before resuming */
    void YieldBriefly();

    /** Called when the yield timer expires — resumes movement */
    void ResumeAfterYield();

};
