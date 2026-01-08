// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

#include "WallTypes.generated.h"

UENUM( BlueprintType )
enum class EWallDirection : uint8
{
	Vertical UMETA( DisplayName = "Vertical" ),
	Horizontal UMETA( DisplayName = "Horizontal" ),
};

/**
 *
 */
USTRUCT( BlueprintType )
struct FWallMeshSet
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Wall" )
	TObjectPtr<UStaticMesh> VerticalMesh = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Wall" )
	TObjectPtr<UStaticMesh> HorizontalMesh = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Wall" )
	TObjectPtr<UStaticMesh> BrokenVerticalMesh = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Wall" )
	TObjectPtr<UStaticMesh> BrokenHorizontalMesh = nullptr;

	// ������� ��� ��� ����������� �����������
	UStaticMesh* GetMeshForDirection( EWallDirection direction ) const;
};