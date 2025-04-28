#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Grid.h"
#include "Components/Button.h"
#include "UnitManager.h"
#include "UnitActionWidget.generated.h"

class UButton;
class AGrid;

UCLASS()
class PAA_API UUnitActionWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	
	virtual void NativeConstruct() override;  // Questa funzione deve esistere per essere usata

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bIsSelectingTile = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	EUnitType SelectedUnitType;

	UPROPERTY()
	class AGrid* GridRef;

	
	//bool bIsSelectingTile = false;
	//EUnitType SelectedUnitType;

	UPROPERTY(meta = (BindWidget))
	UButton* BT_Brawler;

	UPROPERTY(meta = (BindWidget))
	UButton* BT_Sniper;

	UPROPERTY(meta = (BindWidget))
	UButton* BT_Move;

	UPROPERTY(meta = (BindWidget))
	UButton* BT_Attack;

	UPROPERTY(meta = (BindWidget))
	class UBorder* TurnInfoBorder;
	
	UPROPERTY()
	AUnitManager* UnitManager;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* SB_History;

	UPROPERTY(meta = (BindWidget))
	UButton* BT_SkipAttack;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_Message;

	void ShowErrorMessage(const FString& Message);
	void HideErrorMessage();

	UPROPERTY()
	bool bHasSelectedBrawler = false;

	UPROPERTY()
	bool bHasSelectedSniper = false;

	UPROPERTY()
	bool bActionChosenForUnit = false;
	
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VB_PlayerSniper;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VB_PlayerBrawler;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VB_AISniper;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VB_AIBrawler;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_Player_Sniper;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_Player_Brawler;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_AI_Sniper;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_AI_Brawler;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_Player_Sniper;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_Player_Brawler;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_AI_Sniper;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_AI_Brawler;


	
	UFUNCTION(BlueprintCallable)
	void AddHistoryEntry(const FString& Entry);

	bool bIsInitialized = false;

	AGrid* GetGridReference();

	class AGrid* Grid;

	UFUNCTION()
	void OnBrawlerClicked();

	UFUNCTION()
	void OnSniperClicked();

	UFUNCTION()
	void OnMoveClicked();

	UFUNCTION()
	void OnAttackClicked();

	UFUNCTION()
	void OnSkipAttackClicked();
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TurnInfoText;

	UFUNCTION(BlueprintCallable)
	void UpdateTurnText(bool bIsPlayerTurn);

	UFUNCTION(BlueprintCallable)
	void ShowHistoryPanel();

	UFUNCTION()
	void UpdateHealthBar(AUnit* Unit);

	UFUNCTION(BlueprintCallable)
	void ShowHealthBars();


	
	
};
