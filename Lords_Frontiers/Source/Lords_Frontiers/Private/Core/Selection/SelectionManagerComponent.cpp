// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Selection/SelectionManagerComponent.h"

#include "Building/Building.h"
#include "Selectable.h"

// Sets default values for this component's properties
USelectionManagerComponent::USelectionManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void USelectionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void USelectionManagerComponent::SelectSingle( AActor* actor )
{
	if ( !IsValid( actor ) )
	{
		ClearSelection();

		return;
	}

	if ( !actor->Implements<USelectable>() )
	{
		ClearSelection();
		return;
	}

	// ���������, ����� �� ��� ��������
	const bool bCanSelect = ISelectable::Execute_CanBeSelected( actor );
	if ( !bCanSelect )
	{
		ClearSelection();
		return;
	}

	// Re-selecting the already-selected actor changes nothing — don't re-fire
	// selection effects (sound, overlay) or replay the info-window animation.
	if ( ( SelectedActors_.Num() == 1 ) && ( SelectedActors_[0].Get() == actor ) )
	{
		return;
	}

	// Deselect the previous selection without broadcasting; the single
	// OnSelectionChanged below gives listeners one clean transition, so the
	// info window animates for the newly selected object instead of being
	// hidden and re-shown in the same frame.
	for ( const TWeakObjectPtr<AActor>& weakActor : SelectedActors_ )
	{
		if ( AActor* previous = weakActor.Get() )
		{
			if ( previous->Implements<USelectable>() )
			{
				ISelectable::Execute_OnDeselected( previous );
			}
		}
	}
	SelectedActors_.Reset();

	SelectedActors_.Add( actor );

	// �������� OnSelected
	ISelectable::Execute_OnSelected( actor );

	// �����: �������� �����������, ��� ��������� ����������
	OnSelectionChanged.Broadcast();
	// if ( GEngine )
	//{
	//	GEngine->AddOnScreenDebugMessage(
	//	    -1, 1.5f, FColor::Green, FString::Printf( TEXT( "Selected: %s" ),
	//*actor->GetName() )
	//	);
	// }
}

void USelectionManagerComponent::ClearSelection()
{
	const bool bHadSelection = ( SelectedActors_.Num() > 0 );

	for ( const TWeakObjectPtr<AActor>& weakActor : SelectedActors_ )
	{
		if ( AActor* actor = weakActor.Get() )
		{
			if ( actor->Implements<USelectable>() )
			{
				ISelectable::Execute_OnDeselected( actor );
			}
		}
	}

	// ������� ������
	SelectedActors_.Reset();

	if ( bHadSelection )
	{
		OnSelectionChanged.Broadcast();
	}
}

AActor* USelectionManagerComponent::GetPrimarySelectedActor() const
{
	if ( SelectedActors_.Num() == 0 )
	{
		return nullptr;
	}

	// ���� ������ ������� ������� (�� �� ����� ���� ������ ��������� ���������)
	return SelectedActors_[0].Get();
}

ABuilding* USelectionManagerComponent::GetPrimarySelectedBuilding() const
{
	AActor* actor = GetPrimarySelectedActor();
	if ( !actor )
	{
		return nullptr;
	}

	return Cast<ABuilding>( actor );
}
