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

	void IncreaseRefCount();
	void DecreaseRefCount();

	int RefCount() const
	{
		return RefCount_;
	}

private:
	UPROPERTY( EditAnywhere )
	TObjectPtr<UStaticMeshComponent> Mesh_;

	// Number of objects that use this point
	int RefCount_ = 0;
};
