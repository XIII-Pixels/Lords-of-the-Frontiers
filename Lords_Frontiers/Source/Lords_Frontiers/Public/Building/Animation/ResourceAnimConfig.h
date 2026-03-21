// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Resources/GameResource.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ResourceAnimConfig.generated.h"

/**
 * Maxim
 */

USTRUCT( BlueprintType )
struct FResourceCollectionAnimParams
{
	GENERATED_BODY()

	// Общая длительность анимации масштаба здания
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|BuildingScale" )
	float Duration = 1.2f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Animation|BuildingScale",
	    meta = ( ClampMin = "0.0", ClampMax = "1.0" )
	)
	float InflatePhaseEnd = 0.25f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Animation|BuildingScale",
	    meta = ( ClampMin = "0.0", ClampMax = "1.0" )
	)
	float HoldPhaseEnd = 0.75f;

	// Масштаб при максимальном увеличении
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|BuildingScale", meta = ( ClampMin = "1.0" ) )
	float InflateScale = 1.15f;

	// Кривая надвувания сдувания
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|BuildingScale", meta = ( ClampMin = "0.1" ) )
	float EaseExponent = 2.0f;
};
USTRUCT( BlueprintType )
struct FIconPhaseParams
{
	GENERATED_BODY()

	// Стартовый масштаб
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ScaleUp" )
	float InitialScale = 0.1f;

	// Масштаб при максимальном увеличении
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ScaleUp" )
	float FullScale = 1.0f;

	// Продолжительность фазы увеличинения масштаба
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ScaleUp", meta = ( ClampMin = "0.01" ) )
	float ScaleUpDuration = 0.2f;

	// Кривая если больше >1 то медленный старт быстрый финиш
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ScaleUp", meta = ( ClampMin = "0.1" ) )
	float ScaleUpEase = 2.0f;

	// Длительность когда иконка остается в полном масштабе
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hold", meta = ( ClampMin = "0.0" ) )
	float HoldDuration = 0.3f;

	// Длительность фазы отлета
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "FlyAway", meta = ( ClampMin = "0.01" ) )
	float FlyAwayDuration = 0.4f;

	// Насколько сильно иконка уменьшиться
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "FlyAway" )
	float FlyAwayScale = 0.0f;

	// Кривая если больше >1 то медленный старт быстрый финиш
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "FlyAway", meta = ( ClampMin = "0.1" ) )
	float FlyAwayEase = 2.0f;
};

USTRUCT( BlueprintType )
struct FResourceIconRandomParams
{
	GENERATED_BODY()

	// Рандомизация спавна иконок
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Random", meta = ( ClampMin = "0.0" ) )
	float SpawnDelayJitter = 0.1f;

	// Разброс по траектории X
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Random", meta = ( ClampMin = "0.0" ) )
	float TrajectorySpreadX = 15.0f;

	// Разброс по траектории Y
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Random", meta = ( ClampMin = "0.0" ) )
	float TrajectorySpreadY = 10.0f;
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UResourceAnimConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|BuildingScale" )
	FResourceCollectionAnimParams BuildingScaleAnim;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|IconPhases" )
	FIconPhaseParams IconPhases;

	// общая длительность (Если 0 используются таймеры с анимация )
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|IconPhases", meta = ( ClampMin = "0.0" ) )
	float IconAnimDuration = 0.0f;

	// Задержка перед первой иконкой
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Timing", meta = ( ClampMin = "0.0" ) )
	float StartDelay = 0.0f;

	// Задержки появления след иконки
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Timing", meta = ( ClampMin = "0.0" ) )
	float StaggerDelayPerIcon = 0.05f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Random" )
	FResourceIconRandomParams IconRandom;

	// Высота взлета иконки
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Flight", meta = ( ClampMin = "0.0" ) )
	float RiseHeight = 120.0f;

	// Дистанция между икноками
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Flight", meta = ( ClampMin = "0.0" ) )
	float VerticalSpacing = 40.0f;

	// Базовый масштаб
	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Animation|Zoom", meta = ( ClampMin = "0.01", ClampMax = "3.0" )
	)
	float BasePopupScale;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Zoom", meta = ( ClampMin = "1.0" ) )
	float BaseOrthoWidth = 2048.0f;

	// Минимальный размер при зуме
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Zoom", meta = ( ClampMin = "0.01" ) )
	float MinPopupScale = 0.1f;

	// Максимальный размер при зуме
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Zoom", meta = ( ClampMin = "0.1" ) )
	float MaxPopupScale = 2.0f;

	// Когда НЕ показывать анимацию здания
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Zoom", meta = ( ClampMin = "0.0" ) )
	float IconOnlyZoomThreshold = 3500.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Animation|Zoom", meta = ( ClampMin = "0.0", ClampMax = "3.0" )
	)
	float ZoomSpacingExponent = 1.0f;

	// При каком масштабе заменять на икноку
	// 0 = не скрывать текст
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Zoom", meta = ( ClampMin = "0.0" ) )
	float IconOnlyScaleThreshold = 0.35f;

	// Масштаб уменьшения иконки
	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Animation|Zoom", meta = ( ClampMin = "0.01", ClampMax = "3.0" )
	)
	float IconOnlyScaleMultiplier = 0.7f;

	// Распространение волны анимации (за сколько проходит 1см)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Animation|Wave", meta = ( ClampMin = "0.0" ) )
	float WaveDelayPerUnit = 0.002f;
};
