// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/AiPathFollowComponent.h"
#include "AIController.h"
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
    bIsFollowing = true;

    MoveToNextNode();
}

void UAiPathFollowComponent::StopFollowingRoute()
{
    bIsFollowing = false;
    NodeRoute.Empty();
    CurrentNodeIndex = 0;

    if (OwnerController)
    {
        OwnerController->StopMovement();
    }
}

void UAiPathFollowComponent::OnMoveRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    if (!Result.IsSuccess())
    {
		UE_LOG(LogClass, Warning, TEXT("UAiPathFollowComponent::OnMoveRequestFinished - Move request failed or was aborted."));
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


    // Issue MoveToActor in parallel
    if (OwnerController)
    {
		OwnerController->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this); // if we don't do this, we'll bind multiple times.
        OwnerController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &UAiPathFollowComponent::OnMoveRequestFinished);
        OwnerController->MoveToActor(Node, Node->AcceptanceRadius); //
       
    }
}

void UAiPathFollowComponent::OnNodeCompleted()
{
    if (!bIsFollowing)
        return;

    CurrentNodeIndex++;

    if (CurrentNodeIndex < NodeRoute.Num())
    {
        MoveToNextNode();
    }
    else
    {
        // Finished route
        bIsFollowing = false;
    }
}
