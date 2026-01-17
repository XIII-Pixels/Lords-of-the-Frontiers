// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/AttackComponent.h"

#include "Entity.h"

void UAttackComponent::OnRegister()
{
	Super::OnRegister();

	if ( !GetOwner() )
	{
		bOwnerIsValid_ = false;
	}

	OwnerEntity_ = Cast<IEntity>( GetOwner() );
	if ( !OwnerEntity_ )
	{
		bOwnerIsValid_ = false;
	}
}

bool UAttackComponent::OwnerIsValid() const
{
	return bOwnerIsValid_;
}
