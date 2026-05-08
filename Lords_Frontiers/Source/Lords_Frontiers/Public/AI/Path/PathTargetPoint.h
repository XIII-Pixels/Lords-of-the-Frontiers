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

	bool CreateIndependently() const
	{
		return CreateIndependently_;
	}

private:
	UPROPERTY( EditAnywhere )
	TObjectPtr<UStaticMeshComponent> Mesh_;

	UPROPERTY(
	    EditDefaultsOnly, Category = "Settings",
	    meta = ( ToolTip = "If true, a point is created even if there are points of other classes on the cell" )
	)
	bool CreateIndependently_ = false;

	// Number of objects that use this point
	int RefCount_ = 0;
};
