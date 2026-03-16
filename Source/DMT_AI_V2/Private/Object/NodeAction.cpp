#include "Object/NodeAction.h"


void UNodeAction::ExecuteActionAsync(APawn* Pawn, TFunction<void()> OnFinished)
{
    OnFinishedCallback = OnFinished;   
    BP_ExecuteAction(Pawn);            
 }

void UNodeAction::OnFinished()
{
    if (OnFinishedCallback)
    {
        OnFinishedCallback();
        OnFinishedCallback = nullptr; 
    }
}