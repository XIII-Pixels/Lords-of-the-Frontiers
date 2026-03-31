// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTreeNodes/Decorators/IsOnCooldown.h"

#include "AIController.h"
#include "Entity.h"

UIsOnCooldown::UIsOnCooldown()
{
	NodeName = "Is On Cooldown";
}

bool UIsOnCooldown::CalculateRawConditionValue( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory ) const
{
	const AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		UE_LOG( LogTemp, Warning, TEXT( "UIsOnCooldown failed to get AIController" ) );
		return false;
	}

	const auto entity = Cast<IEntity>( controller->GetPawn() );
	if ( !entity )
	{
		UE_LOG( LogTemp, Warning, TEXT( "UIsOnCooldown failed to get IEntity" ) );
		return false;
	}

	return entity->Stats().OnCooldown();
}
