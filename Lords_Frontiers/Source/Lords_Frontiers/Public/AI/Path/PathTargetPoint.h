// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "PathTargetPoint.generated.h"

UCLASS( HideDropdown )
class LORDS_FRONTIERS_API APathTargetPoint : public AActor
{
	GENERATED_BODY()

public:
	APathTargetPoint();

	void Show();
	void Hide();

private:
	UPROPERTY( EditAnywhere )
	TObjectPtr<UStaticMeshComponent> Mesh_;
};
