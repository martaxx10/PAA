
#include "UCoinTossWidget.h"
#include "Components/Button.h"
#include "Grid.h"
#include "UnitManager.h"
#include "Components/TextBlock.h"

void UUCoinTossWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BT_Heads)
		BT_Heads->OnClicked.AddDynamic(this, &UUCoinTossWidget::OnHeadsClicked);

	if (BT_Tails)
		BT_Tails->OnClicked.AddDynamic(this, &UUCoinTossWidget::OnTailsClicked);
}

void UUCoinTossWidget::OnHeadsClicked()
{
	TossCoin(true); // Player ha scelto testa
}

void UUCoinTossWidget::OnTailsClicked()
{
	TossCoin(false); // Player ha scelto croce
}


void UUCoinTossWidget::TossCoin(bool bPlayerChoseHeads)
{
	bool bCoinIsHeads = FMath::RandBool();
	bool bPlayerShouldStarts = (bPlayerChoseHeads == bCoinIsHeads);

	
	FString result = bCoinIsHeads ? TEXT("heads") : TEXT(" tails");
	//FString winner = bPlayerStarts ? TEXT("It's your turn") : TEXT("It's AI's turn");
  
	ResultText->SetText(FText::FromString(FString::Printf(TEXT("Result: %s"), *result)));

	// Delay di 2 secondi prima di iniziare
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, bPlayerShouldStarts]()
	{
		this->RemoveFromParent();  

		if (GridRef)
		{
			GridRef->bPlayerStartsFirst = bPlayerShouldStarts; 
			GridRef->bIsPlayerTurn = bPlayerShouldStarts;
			GridRef->bActionTurnPlayer = bPlayerShouldStarts;
			
			if (bPlayerShouldStarts)
			{
				// 🔹 Tocca al player
				GridRef->bIsPlayerTurn = true;
				
				GridRef->ShowUnitSelectionUI(); // Mostra UI per selezionare unità
			}
			else
			{
				// Tocca all’AI —> spawna immediatamente
				if (GridRef->UnitManager)
				{
					GridRef->UnitManager->SpawnNextAIUnit(GridRef);
				}

				// Poi passa il turno al player
				GridRef->bIsPlayerTurn = true;
				GridRef->ShowUnitSelectionUI();  // Mostra UI al player
			}
		}

	}, 2.0f, false);
}

