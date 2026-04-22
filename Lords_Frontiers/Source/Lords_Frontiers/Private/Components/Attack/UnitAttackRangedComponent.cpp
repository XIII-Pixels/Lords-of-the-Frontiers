// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/UnitAttackRangedComponent.h"

void UUnitAttackRangedComponent::BeginPlay()
{
	Super::BeginPlay();

	SetAttackMode();
	ActivateSight();
}

void UUnitAttackRangedComponent::SetAttackMode()
{
	AttackFilter_ = EAttackFilter::WhatIsOnPath;
}
