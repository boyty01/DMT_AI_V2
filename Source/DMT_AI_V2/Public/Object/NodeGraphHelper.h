// Copyright DMTesseract ltd. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actor/AiPathNode.h"
#include "NodeGraphHelper.generated.h"

UCLASS()
class DMT_AI_V2_API UNodeGraphHelper : public UObject
{
    GENERATED_BODY()

public:
    /** Finds shortest path using Dijkstra from StartNode to GoalNode */
    UFUNCTION(BlueprintCallable, Category = "NodeGraph")
    static TArray<AAiPathNode*> FindShortestPath(AAiPathNode* StartNode, AAiPathNode* GoalNode);

    /** Finds the nearest path node to a world location */
    UFUNCTION(BlueprintCallable, Category = "NodeGraph")
    static AAiPathNode* FindNearestNode(AActor* WorldContext, const FVector& Location);

};