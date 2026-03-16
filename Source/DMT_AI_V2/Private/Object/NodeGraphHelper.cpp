#include "Object/NodeGraphHelper.h"
#include "Containers/Queue.h"
#include "EngineUtils.h" // for TActorIterator

TArray<AAiPathNode*> UNodeGraphHelper::FindShortestPath(AAiPathNode* StartNode, AAiPathNode* GoalNode)
{
    TArray<AAiPathNode*> ResultPath;
    if (!StartNode || !GoalNode)
    {
        return ResultPath;
    }

    // Maps node to total cost from start
    TMap<AAiPathNode*, float> CostSoFar;
    // Maps node to previous node (for path reconstruction)
    TMap<AAiPathNode*, AAiPathNode*> CameFrom;

    struct FNodeQueueItem
    {
        AAiPathNode* Node;
        float Cost;
    };

    TArray<FNodeQueueItem> PriorityQueue;
    PriorityQueue.Add({ StartNode, 0.f });

    CostSoFar.Add(StartNode, 0.f);
    CameFrom.Add(StartNode, nullptr);

    while (PriorityQueue.Num() > 0)
    {
        // Find node with lowest cost
        PriorityQueue.Sort([](const FNodeQueueItem& A, const FNodeQueueItem& B)
            {
                return A.Cost < B.Cost;
            });

        FNodeQueueItem CurrentItem = PriorityQueue[0];
        PriorityQueue.RemoveAt(0);

        AAiPathNode* CurrentNode = CurrentItem.Node;


        if (CurrentNode == GoalNode)
        {
            // Reconstruct path
            AAiPathNode* Node = GoalNode;
            while (Node)
            {
                ResultPath.Insert(Node, 0);
                Node = CameFrom[Node];
            }
            break;
        }

        // Visit neighbors
        for (AAiPathNode* Neighbor : CurrentNode->ConnectedNodes)
        {
            
            if (!Neighbor) continue;

            float NewCost = CostSoFar[CurrentNode] + FVector::Dist(CurrentNode->GetActorLocation(), Neighbor->GetActorLocation());

            if (!CostSoFar.Contains(Neighbor) || NewCost < CostSoFar[Neighbor])
            {
                CostSoFar.Add(Neighbor, NewCost);
                CameFrom.Add(Neighbor, CurrentNode);
                PriorityQueue.Add({ Neighbor, NewCost });
            }
        }
    }

    return ResultPath;
}

AAiPathNode* UNodeGraphHelper::FindNearestNode(AActor* WorldContext, const FVector& Location)
{
    UWorld* World = WorldContext->GetWorld();
    if (!World) return nullptr;

    AAiPathNode* NearestNode = nullptr;
    float MinDistSq = TNumericLimits<float>::Max();

    for (TActorIterator<AAiPathNode> It(World); It; ++It)
    {
        AAiPathNode* Node = *It;
        if (!Node) continue;

        float DistSq = FVector::DistSquared(Location, Node->GetActorLocation());
        if (DistSq < MinDistSq)
        {
            MinDistSq = DistSq;
            NearestNode = Node;
        }
    }

    return NearestNode;
}
