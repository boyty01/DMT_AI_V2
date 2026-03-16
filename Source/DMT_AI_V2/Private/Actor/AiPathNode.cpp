// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AiPathNode.h"
#include "Object/NodeAction.h"
#include <Components/SplineMeshComponent.h>
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#endif

void AAiPathNode::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

}

void AAiPathNode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);


}

void AAiPathNode::BeginPlay()
{
    Super::BeginPlay();

    ActionComponents.Empty();
    GetComponents<UNodeAction>(ActionComponents);

}


AAiPathNode::AAiPathNode()
{
    PrimaryActorTick.bCanEverTick = false;

 

}

void AAiPathNode::OnArrived(APawn* Pawn, TFunction<void()> OnNodeCompleted)
{
    if (!Pawn)
    {
        OnNodeCompleted();
        return;
    }

    // If node is busy, enqueue the pawn and return
    if (bNodeBusy)
    {
        PawnQueue.Enqueue(TPair<APawn*, TFunction<void()>>(Pawn, OnNodeCompleted));
        return;
    }

    bNodeBusy = true;

    // Filter valid actions and make a copy for safe capture
    TArray<UNodeAction*> ValidActionsCopy;
    for (UNodeAction* Action : ActionComponents)
    {
        if (Action && Action->CanExecute(Pawn))
        {
            ValidActionsCopy.Add(Action);
        }
    }

    // Index must be stored in a heap variable for async safety
    int32* CurrentIndex = new int32(0);

    // Capture copies by value for safety
    AAiPathNode* NodeThis = this;
    APawn* PawnCopy = Pawn;

    // Define the recursive function
    TFunction<void()> RunNextAction;
    RunNextAction = [NodeThis, PawnCopy, ValidActionsCopy, CurrentIndex, OnNodeCompleted, &RunNextAction]() mutable
        {
            // Node or pawn might have been destroyed
            if (!NodeThis || !PawnCopy)
            {
                delete CurrentIndex;
                OnNodeCompleted();
                return;
            }

            if (*CurrentIndex >= ValidActionsCopy.Num())
            {
                // All actions finished
                NodeThis->bNodeBusy = false;
                delete CurrentIndex;

                OnNodeCompleted();

                // Check if there is a queued pawn
                TPair<APawn*, TFunction<void()>> NextPawn;
                if (NodeThis->PawnQueue.Dequeue(NextPawn))
                {
                    NodeThis->OnArrived(NextPawn.Key, NextPawn.Value);
                }

                return;
            }

            // Execute current action
            UNodeAction* Action = ValidActionsCopy[*CurrentIndex];
            (*CurrentIndex)++;
            Action->ExecuteActionAsync(PawnCopy, RunNextAction);
        };

    // Start the action sequence
    RunNextAction();
}

