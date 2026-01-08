// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/BehaviorTreeNodes/Tasks/Debug/PrintTask.h"

UPrintTask::UPrintTask()
{
	NodeName = "Print";
}

EBTNodeResult::Type UPrintTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
{
	GEngine->AddOnScreenDebugMessage( -1, TimeToDisplay, FColor::Red, Message );
	return EBTNodeResult::Succeeded;
}
