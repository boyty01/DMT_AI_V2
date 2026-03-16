// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AiPatrolPath.generated.h"

UCLASS()
class DMT_AI_V2_API AAiPatrolPath : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAiPatrolPath();


protected:

	void OnConstruction(const FTransform& Transform)override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Component")
	TObjectPtr<class USplineComponent> PathSpline;

	UPROPERTY(EditAnywhere, Category="AI Path")
	TArray<class AAiPathNode*> PathPoints;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void RebuildSpline();

};
