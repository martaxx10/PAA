#include "UnitActionWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Grid.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"


void UUnitActionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Trova UnitManager nel mondo di gioco
	UnitManager = Cast<AUnitManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AUnitManager::StaticClass()));

	if (UnitManager)
	{
		UE_LOG(LogTemp, Warning, TEXT(" UnitManager trovato!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" Errore: UnitManager non trovato!"));
	}

	
	UE_LOG(LogTemp, Warning, TEXT(" WidgetTree esiste: %s"), WidgetTree ? TEXT("SÌ") : TEXT("NO"));
	if (WidgetTree)
	{
		BT_Brawler = Cast<UButton>(WidgetTree->FindWidget(FName("BT_Brawler")));
		BT_Sniper = Cast<UButton>(WidgetTree->FindWidget(FName("BT_Sniper")));

		UE_LOG(LogTemp, Warning, TEXT(" BT_Brawler trovato via WidgetTree: %s"), BT_Brawler ? TEXT("SÌ") : TEXT("NO"));
		UE_LOG(LogTemp, Warning, TEXT(" BT_Sniper trovato via WidgetTree: %s"), BT_Sniper ? TEXT("SÌ") : TEXT("NO"));
	}

	BT_Brawler = Cast<UButton>(GetWidgetFromName(TEXT("BT_Brawler")));
	BT_Sniper = Cast<UButton>(GetWidgetFromName(TEXT("BT_Sniper")));


	
	if (BT_Brawler)
	{
		BT_Brawler->OnClicked.AddDynamic(this, &UUnitActionWidget::OnBrawlerClicked);
		UE_LOG(LogTemp, Warning, TEXT(" BT_Brawler collegato a OnBrawlerClicked"));
	}

	else
	{
		UE_LOG(LogTemp, Error, TEXT(" BT_Brawler è NULL"));
	}
	


	if (BT_Sniper)
	{
		BT_Sniper->OnClicked.AddDynamic(this, &UUnitActionWidget::OnSniperClicked);
		UE_LOG(LogTemp, Warning, TEXT(" BT_Sniper collegato a OnSniperClicked"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" BT_Sniper è NULL"));
	}
	
	
	// Trova l'istanza della griglia nel mondo
	Grid = Cast<AGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), AGrid::StaticClass()));

	if (Grid)
	{
		UE_LOG(LogTemp, Warning, TEXT(" UnitActionWidget: Grid trovata con successo! %p"), Grid);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" ERRORE: UnitActionWidget non ha trovato Grid!"));
	}

	
	if (BT_Move)
	{
		BT_Move->OnClicked.AddDynamic(this, &UUnitActionWidget::OnMoveClicked);
		UE_LOG(LogTemp, Warning, TEXT("BT_Move collegato a OnSniperClicked"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" BT_Move è NULL"));
	}


	if (BT_Attack)
	{
		BT_Attack->OnClicked.AddDynamic(this, &UUnitActionWidget::OnAttackClicked);
		UE_LOG(LogTemp, Warning, TEXT("BT_Attack collegato a OnSniperClicked"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" BT_Attack è NULL"));
	}

	if (BT_SkipAttack)
	{
		BT_SkipAttack->OnClicked.AddDynamic(this, &UUnitActionWidget::OnSkipAttackClicked);
		BT_SkipAttack->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SB_History)
	{
		SB_History->SetVisibility(ESlateVisibility::Hidden);
	}

	if (PB_Player_Sniper) PB_Player_Sniper->SetVisibility(ESlateVisibility::Hidden);
	if (PB_Player_Brawler) PB_Player_Brawler->SetVisibility(ESlateVisibility::Hidden);
	if (PB_AI_Sniper) PB_AI_Sniper->SetVisibility(ESlateVisibility::Hidden);
	if (PB_AI_Brawler) PB_AI_Brawler->SetVisibility(ESlateVisibility::Hidden);

	if (TXT_Player_Sniper) TXT_Player_Sniper->SetVisibility(ESlateVisibility::Hidden);
	if (TXT_Player_Brawler) TXT_Player_Brawler->SetVisibility(ESlateVisibility::Hidden);
	if (TXT_AI_Sniper) TXT_AI_Sniper->SetVisibility(ESlateVisibility::Hidden);
	if (TXT_AI_Brawler) TXT_AI_Brawler->SetVisibility(ESlateVisibility::Hidden);

	if (VB_PlayerSniper) VB_PlayerSniper->SetVisibility(ESlateVisibility::Hidden);
	if (VB_PlayerBrawler) VB_PlayerBrawler->SetVisibility(ESlateVisibility::Hidden);
	if (VB_AISniper) VB_AISniper->SetVisibility(ESlateVisibility::Hidden);
	if (VB_AIBrawler) VB_AIBrawler->SetVisibility(ESlateVisibility::Hidden);




	
}

void UUnitActionWidget::UpdateTurnText(bool bIsPlayerTurn)
{
	if (!TurnInfoText || !TurnInfoBorder || !GridRef) return;

	if (!GridRef->bGameStarted)
	{
		TurnInfoBorder->SetVisibility(ESlateVisibility::Hidden);  // Nasconde tutto il blocco
		return;
	}

	// Mostra turno corretto
	FString Text = bIsPlayerTurn ? TEXT("Player's Turn") : TEXT("AI's Turn");
	TurnInfoText->SetText(FText::FromString(Text));
	TurnInfoBorder->SetVisibility(ESlateVisibility::Visible);  // Mostra il blocco
}

void UUnitActionWidget::AddHistoryEntry(const FString& Entry)
{
	if (!SB_History) return;

	UTextBlock* NewEntry = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
	if (NewEntry)
	{
		NewEntry->SetText(FText::FromString(Entry));
		SB_History->AddChild(NewEntry);

		// Scorri in basso alla fine
		SB_History->ScrollToEnd();
	}
}


void UUnitActionWidget::OnBrawlerClicked()
{
	UE_LOG(LogTemp, Warning, TEXT(" OnBrawlerClicked chiamato in C++!"));
    
	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT(" ERRORE: Grid è NULL in OnBrawlerClicked!"));
		return;
	}
	
	if (bHasSelectedBrawler)
	{
		ShowErrorMessage(" You have already selected a Brawler!");
		if (BT_Brawler) BT_Brawler->SetIsEnabled(false);
		return;
	}

	// Tutto ok → procedi
	bHasSelectedBrawler = true;
	HideErrorMessage();
	Grid->UnitWidget->bIsSelectingTile = true;
	Grid->UnitWidget->SelectedUnitType = EUnitType::Brawler;

	UE_LOG(LogTemp, Warning, TEXT(" Brawler selezionato! Modalità selezione attivata."));
}

void UUnitActionWidget::OnSniperClicked()
{
	UE_LOG(LogTemp, Warning, TEXT(" OnSniperClicked chiamato in C++!"));

	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT(" ERRORE: Grid è NULL in OnSniperClicked!"));
		return;
	}
	
	if (bHasSelectedSniper)
	{
		ShowErrorMessage(" You have already selected a Sniper!");
		if (BT_Sniper) BT_Sniper->SetIsEnabled(false);
		return;
	}

	// Tutto ok → procedi
	bHasSelectedSniper = true;
	HideErrorMessage();
	Grid->UnitWidget->bIsSelectingTile = true;
	Grid->UnitWidget->SelectedUnitType = EUnitType::Sniper;

	UE_LOG(LogTemp, Warning, TEXT(" Sniper selezionato! Modalità selezione attivata."));
}



AGrid* UUnitActionWidget::GetGridReference()
{
	return Cast<AGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), AGrid::StaticClass()));
}



void UUnitActionWidget::OnMoveClicked()
{
	UE_LOG(LogTemp, Warning, TEXT(" OnMoveClicked chiamato in C++!"));

	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT(" ERRORE: Grid è NULL in OnMoveClicked!"));
		return;
	}

	if (!Grid->SelectedUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("️ Nessuna unità selezionata per il movimento."));
		return;
	}

	if (bActionChosenForUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT(" Azione già scelta per questa unità."));

		if (TXT_Message)
		{
			TXT_Message->SetText(FText::FromString(TEXT("An action is already selected for this unit!")));
			TXT_Message->SetVisibility(ESlateVisibility::Visible);
		}

		return;
	}

	// ✅ Imposta l’azione come scelta
	bActionChosenForUnit = true;

	// Nascondi eventuali messaggi di errore precedenti
	HideErrorMessage();
	
	// Abilita la fase di azione movimento
	Grid->bIsInActionPhase = true;
	Grid->bIsMoveMode = true;
	Grid->bIsAttackMode = false;

	UE_LOG(LogTemp, Warning, TEXT(" Modalità MOVIMENTO attiva per [%s]"), *Grid->SelectedUnit->GetName());

	// Mostra il range di movimento
	Grid->ShowMovementRangeForSelectedUnit();
}


void UUnitActionWidget::OnAttackClicked()
{
	UE_LOG(LogTemp, Warning, TEXT(" OnAttackClicked chiamato in C++!"));

	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT(" ERRORE: Grid è NULL in OnAttackClicked!"));
		return;
	}

	if (!Grid->SelectedUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT(" Nessuna unità selezionata per l'attacco."));
		return;
	}

	if (bActionChosenForUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("️ Azione già scelta per questa unità."));

		if (TXT_Message)
		{
			TXT_Message->SetText(FText::FromString(TEXT("An action is already selected for this unit!")));
			TXT_Message->SetVisibility(ESlateVisibility::Visible);
		}

		return;
	}

	// Imposta l’azione come scelta
	bActionChosenForUnit = true;

	// Nascondi eventuali messaggi precedenti
	HideErrorMessage();

	// Abilita la fase di azione attacco
	Grid->bIsInActionPhase = true;
	Grid->bIsAttackMode = true;
	Grid->bIsMoveMode = false;

	UE_LOG(LogTemp, Warning, TEXT(" Modalità ATTACCO attiva per [%s]"), *Grid->SelectedUnit->GetName());
	
}


void UUnitActionWidget::OnSkipAttackClicked()
{
	if (GridRef)
	{
		GridRef->HandleSkipAttack(); 
	}
	BT_SkipAttack->SetVisibility(ESlateVisibility::Collapsed);
}

void UUnitActionWidget::ShowErrorMessage(const FString& Message)
{
	if (TXT_Message)
	{
		TXT_Message->SetText(FText::FromString(Message));
		TXT_Message->SetVisibility(ESlateVisibility::Visible);
	}
}

void UUnitActionWidget::HideErrorMessage()
{
	if (TXT_Message)
	{
		TXT_Message->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUnitActionWidget::ShowHistoryPanel()
{
	if (SB_History)
	{
		SB_History->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Warning, TEXT(" ScrollBox visibile!"));
	}

	if (SB_History->GetParent())
	{
		SB_History->GetParent()->SetVisibility(ESlateVisibility::Visible);
	}
	
}

void UUnitActionWidget::UpdateHealthBar(AUnit* Unit)
{
	if (!Unit || !GridRef) return;

	float Percent = 0.0f;
	int MaxHealth = Unit->UnitClassType == EUnitType::Brawler ? 40 : 20;
	Percent = FMath::Clamp((float)Unit->Health / (float)MaxHealth, 0.0f, 1.0f);

	if (GridRef->PlayerUnits.Contains(Unit))
	{
		if (Unit->UnitClassType == EUnitType::Brawler && PB_Player_Brawler)
			PB_Player_Brawler->SetPercent(Percent);
		else if (Unit->UnitClassType == EUnitType::Sniper && PB_Player_Sniper)
			PB_Player_Sniper->SetPercent(Percent);
	}
	else if (GridRef->AIUnits.Contains(Unit))
	{
		if (Unit->UnitClassType == EUnitType::Brawler && PB_AI_Brawler)
			PB_AI_Brawler->SetPercent(Percent);
		else if (Unit->UnitClassType == EUnitType::Sniper && PB_AI_Sniper)
			PB_AI_Sniper->SetPercent(Percent);
	}
}

void UUnitActionWidget::ShowHealthBars()
{
	// Player Sniper
	if (VB_PlayerSniper) VB_PlayerSniper->SetVisibility(ESlateVisibility::Visible);
	if (TXT_Player_Sniper) TXT_Player_Sniper->SetVisibility(ESlateVisibility::Visible);
	if (PB_Player_Sniper) PB_Player_Sniper->SetVisibility(ESlateVisibility::Visible);

	// Player Brawler
	if (VB_PlayerBrawler) VB_PlayerBrawler->SetVisibility(ESlateVisibility::Visible);
	if (TXT_Player_Brawler) TXT_Player_Brawler->SetVisibility(ESlateVisibility::Visible);
	if (PB_Player_Brawler) PB_Player_Brawler->SetVisibility(ESlateVisibility::Visible);

	// AI Sniper
	if (VB_AISniper) VB_AISniper->SetVisibility(ESlateVisibility::Visible);
	if (TXT_AI_Sniper) TXT_AI_Sniper->SetVisibility(ESlateVisibility::Visible);
	if (PB_AI_Sniper) PB_AI_Sniper->SetVisibility(ESlateVisibility::Visible);

	// AI Brawler
	if (VB_AIBrawler) VB_AIBrawler->SetVisibility(ESlateVisibility::Visible);
	if (TXT_AI_Brawler) TXT_AI_Brawler->SetVisibility(ESlateVisibility::Visible);
	if (PB_AI_Brawler) PB_AI_Brawler->SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogTemp, Warning, TEXT(" Barre della vita rese visibili!"));
}

