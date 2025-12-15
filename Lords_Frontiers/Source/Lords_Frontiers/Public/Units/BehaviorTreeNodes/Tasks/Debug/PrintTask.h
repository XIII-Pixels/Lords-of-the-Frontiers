// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "PrintTask.generated.h"

/** (Gregory-hub)
 * 
 */
UCLASS()
class LORDS_FRONTIERS_API UPrintTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UPrintTask();

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
	
	UPROPERTY( EditAnywhere, Category = "Settings" )
	FString Message = "Default PrintTask message";
	
	UPROPERTY( EditAnywhere, Category = "Settings" )
	float TimeToDisplay = 2.0f;
};
