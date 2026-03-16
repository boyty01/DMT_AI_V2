// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AiPatrolPath.h"
#include "Components/SplineComponent.h"
#include "Actor/AiPathNode.h"

// Sets default values
AAiPatrolPath::AAiPatrolPath()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	PathSpline->SetupAttachment(RootComponent);
}

void AAiPatrolPath::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildSpline();
}

// Called when the game starts or when spawned
void AAiPatrolPath::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAiPatrolPath::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAiPatrolPath::RebuildSpline()
{
	if (!PathSpline) return;

	PathSpline->ClearSplinePoints();

	for (AAiPathNode* Point : PathPoints)
	{
		if (Point)
		{
			PathSpline->AddSplinePoint(Point->GetActorLocation(), ESplineCoordinateSpace::World);
		}
	}
}

