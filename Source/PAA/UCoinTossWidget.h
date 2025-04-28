

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
class AGrid;
#include "Components/Button.h"
#include "UCoinTossWidget.generated.h"



UCLASS()
class PAA_API UUCoinTossWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	class UButton* BT_Heads;

	UPROPERTY(meta=(BindWidget))
	class UButton* BT_Tails;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ResultText;

	UFUNCTION()
	void OnHeadsClicked();

	UFUNCTION()
	void OnTailsClicked();

	UPROPERTY()
	AGrid* GridRef;


private:
	void TossCoin(bool bPlayerChoseHeads); // true = heads, false = tails

	bool bPlayerStarts;

	
};
