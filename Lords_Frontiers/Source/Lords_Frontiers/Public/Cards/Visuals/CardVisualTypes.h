#pragma once

#include "CoreMinimal.h"

#include "CardVisualTypes.generated.h"

class UMaterialInterface;
class UNiagaraSystem;
class UTexture2D;

UENUM( BlueprintType )
enum class ECardVisualTarget : uint8
{
	None	UMETA( DisplayName = "Не показывать" ),
	Owner	UMETA( DisplayName = "На носителе эффекта" ),
	Target	UMETA( DisplayName = "На цели" ),
	Both	UMETA( DisplayName = "На носителе и цели" ),
};

UENUM( BlueprintType )
enum class ECardVisualIconMode : uint8
{
	Popup	UMETA( DisplayName = "Всплывающая (одноразовая)" ),
	Sticky	UMETA( DisplayName = "Постоянная (пока эффект активен)" ),
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardNiagaraSpec
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Niagara-система" ) )
	TSoftObjectPtr<UNiagaraSystem> System;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Где играть",
			ToolTip = "На каком актёре спавнить систему: на носителе эффекта (для статусов — горящий враг; для карт башни — сама башня), на цели (для AoE-взрыва — каждый задетый враг; для попадания — жертва) или на обоих." ) )
	ECardVisualTarget Anchor = ECardVisualTarget::Owner;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Зациклить",
			ToolTip = "Если включено, эффект играет цикл, пока активен карточный эффект; иначе — одиночный спавн." ) )
	bool bLoop = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Задержка (сек)",
			ClampMin = "0.0",
			ToolTip = "Сколько секунд подождать перед спавном. Удобно для синхронизации с анимацией / звуком." ) )
	float DelaySeconds = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Смещение позиции",
			ToolTip = "Локальное смещение относительно точки спавна." ) )
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Масштаб",
			ToolTip = "Масштаб (по осям), применяемый к Niagara-компоненту при спавне. Также пробрасывается в User-параметр Niagara, см. поле ниже." ) )
	FVector Scale = FVector::OneVector;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Имя User-параметра для масштаба",
			ToolTip = "Имя float User-параметра внутри Niagara System, в который нужно записать Scale.X при спавне. Пусто — не пробрасывать. Префикс \"User.\" добавляется автоматически. Дефолт — Scale_FX." ) )
	FName ScaleUserParameter = FName( "Scale_FX" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Прикреплять к актёру",
			ToolTip = "Прикреплять Niagara-компонент к актёру (башне/врагу), а не спавнить отдельно в мире." ) )
	bool bAttachToHost = true;

	// Attach Niagara to the host's USkeletalMeshComponent so emitters with
	// "Sample Skeletal Mesh" data interfaces auto-bind (particles spawn on bones).
	// Falls back to the root component if no skeletal mesh is found.
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( DisplayName = "Привязать к скелетному мешу",
			EditCondition = "bAttachToHost",
			ToolTip = "Прикреплять к SkeletalMeshComponent, чтобы эмиттеры с Sample Skeletal Mesh цеплялись к костям." ) )
	bool bAttachToSkeletalMesh = false;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardIconSpec
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon",
		meta = ( DisplayName = "Иконка" ) )
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon",
		meta = ( DisplayName = "Где показывать",
			ToolTip = "На каком актёре показать иконку: на носителе эффекта, на цели или на обоих." ) )
	ECardVisualTarget ShowOn = ECardVisualTarget::Owner;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon",
		meta = ( DisplayName = "Режим" ) )
	ECardVisualIconMode Mode = ECardVisualIconMode::Popup;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon",
		meta = ( DisplayName = "Задержка (сек)",
			ClampMin = "0.0" ) )
	float DelaySeconds = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon|Popup",
		meta = ( DisplayName = "Длительность всплытия (сек)",
			ClampMin = "0.05", EditCondition = "Mode == ECardVisualIconMode::Popup", EditConditionHides ) )
	float PopupDurationSeconds = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon|Popup",
		meta = ( DisplayName = "Высота всплытия",
			EditCondition = "Mode == ECardVisualIconMode::Popup", EditConditionHides ) )
	float PopupFloatHeight = 120.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon",
		meta = ( DisplayName = "Базовое смещение по высоте" ) )
	float BaseHeightOffset = 0.f;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardOverlayMaterialSpec
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Overlay",
		meta = ( DisplayName = "Материал" ) )
	TSoftObjectPtr<UMaterialInterface> Material;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Overlay",
		meta = ( DisplayName = "Где применять" ) )
	ECardVisualTarget ApplyOn = ECardVisualTarget::None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Overlay",
		meta = ( DisplayName = "Задержка (сек)",
			ClampMin = "0.0" ) )
	float DelaySeconds = 0.f;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardVisualConfig
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals",
		meta = ( DisplayName = "Niagara-эффекты",
			ToolTip = "Список Niagara-эффектов. Каждый настраивается независимо (где играть, масштаб, задержка)." ) )
	TArray<FCardNiagaraSpec> Niagaras;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals",
		meta = ( DisplayName = "Иконки",
			ToolTip = "Список иконок (всплывающие и постоянные). Каждая настраивается независимо." ) )
	TArray<FCardIconSpec> Icons;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals",
		meta = ( DisplayName = "Оверлей" ) )
	FCardOverlayMaterialSpec Overlay;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardVisualHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Id = INDEX_NONE;

	bool IsValid() const
	{
		return Id != INDEX_NONE;
	}

	void Reset()
	{
		Id = INDEX_NONE;
	}

	friend bool operator==( const FCardVisualHandle& a, const FCardVisualHandle& b )
	{
		return a.Id == b.Id;
	}

	friend uint32 GetTypeHash( const FCardVisualHandle& h )
	{
		return ::GetTypeHash( h.Id );
	}
};
