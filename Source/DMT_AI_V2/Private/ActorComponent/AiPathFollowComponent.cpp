// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/AiPathFollowComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Actor/AiPathNode.h"
#include "GameFramework/Pawn.h"



UAiPathFollowComponent::UAiPathFollowComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentNodeIndex = 0;
    bIsFollowing = false;
    OwnerPawn = nullptr;
    OwnerController = nullptr;
}

void UAiPathFollowComponent::FollowNodeRoute(const TArray<AAiPathNode*>& Route)
{
    if (Route.Num() == 0)
    {
        return;

    }

    OwnerPawn = Cast<APawn>(GetOwner());
    
    if (!OwnerPawn)
    {
        return;
    }
    OwnerController = Cast<AAIController>(OwnerPawn->GetController());
    if (!OwnerController)
    {
        return;
    }
     

    NodeRoute = Route;
    CurrentNodeIndex = 0;
    CurrentRetryCount = 0;
    bIsYielding = false;
    bIsPaused = false;
    bIsFollowing = true;

    MoveToNextNode();
}

void UAiPathFollowComponent::StopFollowingRoute()
{
    bIsFollowing = false;
    bIsYielding = false;
    bIsPaused = false;
    NodeRoute.Empty();
    CurrentNodeIndex = 0;
    CurrentRetryCount = 0;

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RetryTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(StuckCheckTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(YieldTimerHandle);
    }

    if (OwnerController)
    {
        OwnerController->StopMovement();
    }
}

void UAiPathFollowComponent::PausePathFollow()
{
    if (!bIsFollowing || bIsPaused) return;

    bIsFollowing = false;
    bIsPaused = true;
    bIsYielding = false;

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RetryTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(StuckCheckTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(YieldTimerHandle);
    }

    if (OwnerController)
    {
        OwnerController->StopMovement();
    }
}

void UAiPathFollowComponent::ResumePathFollow()
{
    if (!bIsPaused) return;

    bIsPaused = false;
    bIsFollowing = true;
    CurrentRetryCount = 0;

    MoveToNextNode();
}

void UAiPathFollowComponent::OnMoveRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    // Move has concluded — stop sampling position regardless of outcome
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(StuckCheckTimerHandle);
    }

    if (!Result.IsSuccess())
    {
        if (!bIsFollowing) return;

        CurrentRetryCount++;
        if (CurrentRetryCount <= MaxMoveRetries)
        {
            UE_LOG(LogClass, Warning, TEXT("UAiPathFollowComponent: Move failed for node %d. Retry %d/%d in %.1fs."), CurrentNodeIndex, CurrentRetryCount, MaxMoveRetries, RetryDelay);
            GetWorld()->GetTimerManager().SetTimer(RetryTimerHandle, this, &UAiPathFollowComponent::RetryCurrentNode, RetryDelay, false);
        }
        else
        {
            UE_LOG(LogClass, Warning, TEXT("UAiPathFollowComponent: Max retries exceeded for node %d — skipping."), CurrentNodeIndex);
            CurrentRetryCount = 0;
            CurrentNodeIndex++;
            MoveToNextNode();
        }
        return;
    }

    if (!NodeRoute.IsValidIndex(CurrentNodeIndex)) return;

    AAiPathNode* PendingNode = NodeRoute[CurrentNodeIndex];

    // NOW we have actually arrived
    if (PendingNode)
    {
        PendingNode->OnArrived(OwnerPawn, [this]()
            {
                OnNodeCompleted();
            });
    }
}

void UAiPathFollowComponent::MoveToNextNode()
{
    if (!bIsFollowing || CurrentNodeIndex >= NodeRoute.Num())
    {
        return;
    }
     

    AAiPathNode* Node = NodeRoute[CurrentNodeIndex];
    if (!Node)
    {
        CurrentNodeIndex++;
        MoveToNextNode();
        return;
    }


    // Issue MoveToActor
    if (OwnerController)
    {
		UPathFollowingComponent* PFC = OwnerController->GetPathFollowingComponent();
		if (PFC)
		{
			PFC->OnRequestFinished.RemoveAll(this); // if we don't do this, we'll bind multiple times.
			PFC->OnRequestFinished.AddUObject(this, &UAiPathFollowComponent::OnMoveRequestFinished);
		}
        OwnerController->MoveToActor(Node, Node->AcceptanceRadius);

        // Begin stuck detection for this move
        if (OwnerPawn && GetWorld())
        {
            LastStuckCheckLocation = OwnerPawn->GetActorLocation();
            GetWorld()->GetTimerManager().SetTimer(StuckCheckTimerHandle, this, &UAiPathFollowComponent::CheckIfStuck, StuckCheckInterval, true);
        }
    }
}

void UAiPathFollowComponent::RetryCurrentNode()
{
    if (bIsFollowing)
    {
        MoveToNextNode();
    }
}

void UAiPathFollowComponent::CheckIfStuck()
{
    if (!bIsFollowing || bIsYielding || !OwnerPawn) return;

    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    float DistanceMoved = FVector::Dist2D(CurrentLocation, LastStuckCheckLocation);
    LastStuckCheckLocation = CurrentLocation;

    if (DistanceMoved < StuckDistanceThreshold)
    {
        YieldBriefly();
    }
}

void UAiPathFollowComponent::YieldBriefly()
{
    if (!OwnerController || !GetWorld()) return;

    bIsYielding = true;
    GetWorld()->GetTimerManager().ClearTimer(StuckCheckTimerHandle);
    OwnerController->StopMovement();

    // Randomise the yield duration so two NPCs stuck against each other
    // will yield for different lengths and one gets ahead of the other.
    const float ActualYield = YieldDuration + FMath::FRandRange(0.0f, YieldDuration * 0.5f);
    GetWorld()->GetTimerManager().SetTimer(YieldTimerHandle, this, &UAiPathFollowComponent::ResumeAfterYield, ActualYield, false);
}

void UAiPathFollowComponent::ResumeAfterYield()
{
    bIsYielding = false;
    if (bIsFollowing)
    {
        MoveToNextNode();
    }
}

void UAiPathFollowComponent::OnNodeCompleted()
{
    if (!bIsFollowing)
        return;

    CurrentRetryCount = 0;
    CurrentNodeIndex++;

    if (CurrentNodeIndex < NodeRoute.Num())
    {
        // Defer to next tick — MoveToActor must not be called from inside an
        // OnRequestFinished callback or the PFC hasn't reset its state yet,
        // which causes the new request to abort immediately and skip nodes.
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UAiPathFollowComponent::MoveToNextNode);
    }
    else
    {
        // Finished route
        bIsFollowing = false;
    }
}
