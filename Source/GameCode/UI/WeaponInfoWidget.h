#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponInfoWidget.generated.h"

class UTextBlock;

UCLASS()
class GAMECODE_API UWeaponInfoWidget : public UUserWidget
{
	GENERATED_BODY()

friend class UPlayerHUDWidget;
	
public:
	void SetAmmo(int32 CurrentAmmo, int32 TotalAmmo);
	void SetThrowablesCount(int32 ThrowablesCount);
	void OnMeleeWeaponEquipped();

protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* ActiveWeaponNameTextblock;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* ActiveThrowableNameTextblock;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UWidget* AmmoWidgetsContainerWidget;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* ClipAmmoTextblock;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* RemainingAmmoTextblock;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* RemainingThrowablesTextblock;
};
