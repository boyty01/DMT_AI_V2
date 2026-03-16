// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "AiPathNode.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNodeReached, AAiPathNode*, Node, AActor*, ArrivedActor);

class UNodeAction;
class USplineMeshComponent;

UCLASS(Blueprintable)
class DMT_AI_V2_API AAiPathNode : public AActor
{
	GENERATED_BODY()

public:

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;


	// Sets default values for this actor's properties
	AAiPathNode();

	// how close can the pawn be before the node is considered reached.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Path Node")
	float AcceptanceRadius{ 50.f };

	// Connected nodes for Dikjstra path finding.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Node")
	TArray<AAiPathNode*> ConnectedNodes;

	// actions that should be triggered on reaching this node.
	TArray<UNodeAction*> ActionComponents;

	// Can be used for flagging if the node is a destination node. Cinematic mark etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Node")
	bool bIsGoalNode;

	void OnArrived(APawn* Pawn, TFunction<void()> OnNodeCompleted);

	UPROPERTY(BlueprintReadOnly)
	bool bNodeBusy{ false };

protected:



private:

	TQueue<TPair<APawn*, TFunction<void()>>> PawnQueue;


};

