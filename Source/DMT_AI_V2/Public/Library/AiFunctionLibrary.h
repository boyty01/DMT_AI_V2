// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AiFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class DMT_AI_V2_API UAiFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Ai")
	static bool GetActorAiScript(AActor* TargetActor, UDmtAiV2ScriptBase*& Script);

	UFUNCTION(BlueprintCallable, Category = "Ai")
	static bool GetDmtAiBehaviorTarget(AActor* TargetActor, AActor*& BehaviorTarget);

	UFUNCTION(BlueprintCallable, Category ="Ai")
	static bool GetDmtAiBehaviorComponent(AActor* TargetActor, UDmtAiBehaviorComponent*& BehaviorComponent);
};
