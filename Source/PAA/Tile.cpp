#include "Tile.h"
#include "UnitManager.h"
#include "UnitActionWidget.h"
#include "Kismet/GameplayStatics.h"


// Costruttore
ATile::ATile()
{
	PrimaryActorTick.bCanEverTick = false;
	
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	RootComponent = TileMesh;
	
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (MeshAsset.Succeeded())
	{
		TileMesh->SetStaticMesh(MeshAsset.Object);
	}

	// Mtaeriali
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (MaterialAsset.Succeeded())
	{
		DefaultMaterial = MaterialAsset.Object;
		TileMesh->SetMaterial(0, MaterialAsset.Object);
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HighlightMatAsset(TEXT("/Game/Materials/M_Highlight.M_Highlight"));
	if (HighlightMatAsset.Succeeded())
	{
		HighlightedMaterial = HighlightMatAsset.Object;
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" HighlightedMaterial NON trovato!"));
	}


	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AttackMat(TEXT("/Game/Materials/M_AttackHighlight"));
	if (AttackMat.Succeeded())
	{
		AttackHighlightMaterial = AttackMat.Object;
	}

	
	SetActorLocation(GetActorLocation() + FVector(0, 0, 50.0f));
	
	TileMesh->SetWorldScale3D(FVector(1.5f, 1.5f, 1.0f)); // Aumentiamo leggermente la grandezza

	TileMesh->OnClicked.AddDynamic(this, &ATile::OnTileClicked);

	TileMesh->SetCollisionObjectType(ECC_WorldDynamic);
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	TileMesh->SetGenerateOverlapEvents(true);

	// Abilita l'input per l'attore
	AutoReceiveInput = EAutoReceiveInput::Player0;
	
}

void ATile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Restituisce la cella 
FString ATile::GetTileCoordinateLabel() const
{
	TCHAR Letter = 'A' + GridPosition.X;
	int32 Number = GridPosition.Y + 1;
	return FString::Printf(TEXT("%c%d"), Letter, Number);
}

// Imposta celle come ostacolo
void ATile::SetObstacle(bool bNewState, UMaterialInterface* ObstacleMat)
{
	bIsObstacle = bNewState;

	if (bIsObstacle)
	{
		if (ObstacleMat)
		{
			TileMesh->SetMaterial(0, ObstacleMat);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATile: ERRORE! Tentativo di assegnare un materiale NULL agli ostacoli!"));
		}
		
		TileMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else
		TileMesh->SetMaterial(0, DefaultMaterial);
		TileMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

}


void ATile::OnTileClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	
    AGrid* Grid = Cast<AGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), AGrid::StaticClass()));
	UE_LOG(LogTemp, Warning, TEXT(" ATile::OnTileClicked chiamato! Cella [%s]"), *GetTileCoordinateLabel());

    if (!Grid)
    {
        UE_LOG(LogTemp, Error, TEXT(" ERRORE: AGrid non trovato!"));
        return;
    }

    // Blocco il click sulla tile appena usata per spawn
    if (!Grid->bGameStarted && this == Grid->LastSpawnedTile)
    {
        UE_LOG(LogTemp, Warning, TEXT(" Click ignorato subito dopo lo spawn!"));
        //Grid->LastSpawnedTile = nullptr;
        return;
    }

	// ️ Attacco attivo: se clicco su una tile valida, inizia il movimento

	if (Grid->bGameStarted && Grid->bIsPlayerTurn && Grid->bIsInActionPhase && Grid->bIsAttackMode && Grid->SelectedUnit)
	{
		AUnit* UnitFoundOnTile = nullptr;

		// Trova se c'è una unità nemica su questa tile
		for (AUnit* Unit : Grid->AIUnits)
		{
			if (Unit && Unit->CurrentTile == this)
			{
				UnitFoundOnTile = Unit;
				break;
			}
		}

		if (AUnit* TargetUnit = UnitFoundOnTile)
		{
			if (Grid->SelectedUnit->IsInAttackRange(TargetUnit))
			{
				int Damage = Grid->SelectedUnit->CalculateDamage();
				TargetUnit->Health -= Damage;

				// Controattacco
				if (Grid->SelectedUnit->UnitType == EUnitType::Sniper)
				{
					bool bShouldCounterAttack = false;

					if (TargetUnit->UnitType == EUnitType::Sniper)
					{
						bShouldCounterAttack = true;
					}
					else if (TargetUnit->UnitType == EUnitType::Brawler)
					{
						int32 Distance = FMath::Abs(Grid->SelectedUnit->CurrentTile->GridPosition.X - TargetUnit->CurrentTile->GridPosition.X)
									   + FMath::Abs(Grid->SelectedUnit->CurrentTile->GridPosition.Y - TargetUnit->CurrentTile->GridPosition.Y);

						if (Distance == 1)
						{
							bShouldCounterAttack = true;
						}
					}

					if (bShouldCounterAttack)
					{
						int32 CounterDamage = FMath::RandRange(1, 3);
						Grid->SelectedUnit->Health -= CounterDamage;

						if (Grid->UnitWidget)
						{
							Grid->UnitWidget->UpdateHealthBar(Grid->SelectedUnit);
						}

						UE_LOG(LogTemp, Warning, TEXT(" Contrattacco subito! Danno: %d"), CounterDamage);

						if (Grid->SelectedUnit->Health <= 0)
						{
							UE_LOG(LogTemp, Warning, TEXT("️ Lo Sniper è stato ucciso dal contrattacco!"));
							Grid->PlayerUnits.Remove(Grid->SelectedUnit);
							Grid->SelectedUnit->Destroy();
							Grid->SelectedUnit = nullptr;
							Grid->CheckGameOver();
							return;
						}
					}
				}


				if (Grid && Grid->UnitWidget)
				{
					Grid->UnitWidget->UpdateHealthBar(TargetUnit);
				}

				
				// Log attacco del Player nella cronologia
				FString TeamCode = TEXT("HP");
				FString TypeCode = TEXT("?");
				if (Grid->SelectedUnit)
				{
					switch (static_cast<uint8>(Grid->SelectedUnit->UnitType))
					{
					case static_cast<uint8>(EUnitType::Brawler):
						TypeCode = TEXT("B");
						break;
					case static_cast<uint8>(EUnitType::Sniper):
						TypeCode = TEXT("S");
						break;
					default:
						TypeCode = TEXT("?");
						break;
					}
				}
				FString TargetCoord = TargetUnit->CurrentTile->GetTileCoordinateLabel();
				FString AttackEntry = FString::Printf(TEXT("%s: %s %s %d"), *TeamCode, *TypeCode, *TargetCoord, Damage);
				Grid->AddMoveToHistory(AttackEntry);


				
				Grid->SelectedUnit->bHasActedThisTurn = true;


				UE_LOG(LogTemp, Warning, TEXT(" Attacco riuscito! Danno: %d"), Damage);
				UE_LOG(LogTemp, Warning, TEXT(" HP rimanenti: %d"), TargetUnit->Health);

				// Segna come "unità ha agito"
				Grid->SelectedUnit->bHasActedThisTurn = true;

				if (TargetUnit->Health <= 0)
				{
					TargetUnit->Destroy();
					Grid->AIUnits.Remove(TargetUnit);
					UE_LOG(LogTemp, Warning, TEXT("️ Unità eliminata!"));

					Grid->CheckGameOver();
				}


				// Deseleziona unità e rimuove highlight sempre
				Grid->SelectedUnit = nullptr;
				Grid->SelectedTile = nullptr;
				ATile::ClearAllTileHighlights(Grid->TileGrid);
				Grid->bIsInActionPhase = false;
				Grid->bIsAttackMode = false;
				Grid->bIsMoveMode = false;

				if (Grid->UnitWidget)
				{
					Grid->UnitWidget->bActionChosenForUnit = false;
				}
				
				// Verifica se il turno può finire
				bool bAllUnitsActed = true;
				for (AUnit* PlayerUnit : Grid->PlayerUnits)
				{
					if (PlayerUnit && !PlayerUnit->bHasActedThisTurn)
					{
						bAllUnitsActed = false;
						break;
					}
				}

				if (bAllUnitsActed)
				{
					Grid->bIsPlayerTurn = false;
					if (Grid->UnitWidget)
						Grid->UnitWidget->UpdateTurnText(false);

					FTimerHandle TimerHandle;
					Grid->GetWorld()->GetTimerManager().SetTimer(TimerHandle, [Grid]() {
						Grid->HandleAITurn();
					}, 0.5f, false);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT(" Unità ha attaccato, ma il turno continua per l'altra unità."));
				}
				return;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT(" Unità fuori dal range di attacco."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(" Nessuna unità nemica sulla tile selezionata."));
		}

		return;
	}
	
	// Movimento attivo: se clicco su una tile valida, inizia il movimento
	if (Grid->bGameStarted && Grid->bIsPlayerTurn && Grid->bIsInActionPhase && Grid->bIsMoveMode && Grid->SelectedUnit)
	{
		TArray<ATile*> Path = Grid->FindPathBFS(Grid->SelectedUnit->CurrentTile, this);

		if (Path.Num() > 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("️ Percorso trovato con %d step verso [%s]"), Path.Num(), *GetTileCoordinateLabel());
			Grid->MoveUnitAlongPath(Grid->SelectedUnit, Path);
			Grid->ClearMovementRange();
			Grid->SelectedUnit = nullptr;
			Grid->SelectedTile = nullptr;
			Grid->bIsInActionPhase = false;
			Grid->bIsMoveMode = false;

			//  Dopo il movimento del Player → AI muove automaticamente
			if (Grid->bGameStarted && !Grid->bIsPlayerTurn)
			{
				UE_LOG(LogTemp, Warning, TEXT(" Chiamo HandleAITurn() dopo movimento del Player"));
				Grid->HandleAITurn();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(" Nessun percorso valido verso [%s]"), *GetTileCoordinateLabel());
		}
		return;
	}
	
    
	if (Grid->bGameStarted && Grid->bIsPlayerTurn)
	{
		for (AUnit* Unit : Grid->PlayerUnits)
		{
			if (Unit && Unit->CurrentTile)
			{
				if (Unit == Grid->SelectedUnit)
				{
					if (Grid->SelectedTile)
					{
						Grid->SelectedTile->SetHighlight(false);
					}
					Grid->SelectedUnit = nullptr;
					Grid->SelectedTile = nullptr;
					Grid->ClearMovementRange();

					UE_LOG(LogTemp, Warning, TEXT(" Deselezione unità e rimozione del range di movimento."));
					return;
				}

				if (Unit->CurrentTile == this)
				{
					
					if (Grid->SelectedTile)
						Grid->SelectedTile->SetHighlight(false);

					Grid->SelectedUnit = Unit;
					Grid->SelectedTile = this;
					SetHighlight(true);

					UE_LOG(LogTemp, Warning, TEXT(" Unità selezionata manualmente sulla cella [%s]!"), *GetTileCoordinateLabel());

					Grid->ShowMovementRangeForSelectedUnit();

					return;
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT(" Click ignorato: non hai cliccato su una tua unità."));
		return;
	}

    //  Fase di selezione tile per lo spawn
    if (!Grid->UnitWidget || !Grid->UnitWidget->bIsSelectingTile)
    {
        UE_LOG(LogTemp, Warning, TEXT(" Click ignorato: non in fase di selezione tile."));
        return;
    }

    //  L'unità è stata selezionata per lo spawn
    UE_LOG(LogTemp, Warning, TEXT(" Unità selezionata per lo spawn su [%s]"), *GetTileCoordinateLabel());

    FIntPoint TilePosition = Grid->GetTilePosition(this);

    if (Grid->bIsPlayerTurn && Grid->PlayerUnitsToSpawn > 0)
    {
        Grid->UnitManager->SpawnPlayerUnit(Grid, Grid->UnitWidget->SelectedUnitType, TilePosition);
    }

    if (Grid->PlayerUnits.Num() == 2 && Grid->AIUnits.Num() == 2 && !Grid->bGameStarted)
    {
    	Grid->bGameStarted = true;
    	Grid->bIsPlayerTurn = Grid->bPlayerStartsFirst;
    	Grid->bActionTurnPlayer = Grid->bPlayerStartsFirst;
    	
    	// Mostra la scritta solo ora che il gioco è iniziato
    	if (Grid->UnitWidget)
    	{
    		Grid->UnitWidget->UpdateTurnText(Grid->bIsPlayerTurn);
    		Grid->UnitWidget->ShowHistoryPanel();
    		Grid->UnitWidget->ShowHealthBars();
    		
    	}

    	// Nasconde i widget dei turni quando ho posizionato le unità
    	if (Grid->UnitWidget)
    	{
    		if (Grid->UnitWidget->BT_Brawler)
    			Grid->UnitWidget->BT_Brawler->SetVisibility(ESlateVisibility::Hidden);
    
    		if (Grid->UnitWidget->BT_Sniper)
    			Grid->UnitWidget->BT_Sniper->SetVisibility(ESlateVisibility::Hidden);
    	}
    	
    	if (Grid->bIsPlayerTurn)
    	{
    		Grid->ShowUnitSelectionUI();
    		UE_LOG(LogTemp, Warning, TEXT(" Inizio della partita! Tocca al Player"));
    	}
    	else
    	{
    		UE_LOG(LogTemp, Warning, TEXT(" Inizio della partita! Tocca all'AI"));
    		Grid->HandleAITurn();
    	}
    	
    }
}



void ATile::SetHighlighted(bool bHighlighted)
{
	SetHighlight(bHighlighted);
}


void ATile::SetHighlight(bool bEnable)
{
	if (bIsObstacle) return;
	
	if (!TileMesh) return;

	if (bEnable && HighlightedMaterial)
	{
		TileMesh->SetMaterial(0, HighlightedMaterial);
	}
	else if (DefaultMaterial)
	{
		TileMesh->SetMaterial(0, DefaultMaterial);
	}
}


void ATile::ClearAllTileHighlights(const TArray<TArray<ATile*>>& TileGrid)
{
	for (const TArray<ATile*>& Row : TileGrid)
	{
		for (ATile* Tile : Row)
		{
			if (Tile)
			{
				Tile->SetHighlight(false);
				Tile->SetAttackHighlight(false);  
			}
		}
	}
}

void ATile::SetAttackHighlight(bool bEnable)
{
	bIsAttackHighlighted = bEnable;

	if (!TileMesh || bIsObstacle) return;

	if (bEnable && AttackHighlightMaterial)
	{
		TileMesh->SetMaterial(0, AttackHighlightMaterial);
	}
	else if (!bEnable && DefaultMaterial)
	{
		TileMesh->SetMaterial(0, DefaultMaterial);
	}
}



void ATile::ShowMovementRange(AUnit* Unit, int32 MaxRange)
{
	if (!Unit || !Unit->CurrentTile) return;

	// Salta se questa tile è un ostacolo!
	if (bIsObstacle) return;

	FIntPoint Start = Unit->CurrentTile->GridPosition;
	int Distance = FMath::Abs(Start.X - GridPosition.X) + FMath::Abs(Start.Y - GridPosition.Y);

	if (Distance <= MaxRange)
	{
		SetHighlight(true);
		UE_LOG(LogTemp, Warning, TEXT(" Tile [%s] evidenziata per movimento."), *GetTileCoordinateLabel());
	}

	
}


void ATile::ClearHighlightIfNotSelected()
{
	// Non rimuovere highlight se è la cella della unità selezionata
	AGrid* Grid = Cast<AGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), AGrid::StaticClass()));
	if (Grid && Grid->SelectedTile == this)
		return;

	SetHighlight(false);
}



void ATile::OnTileMeshClicked(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Warning, TEXT(" ATile::OnTileMeshClicked chiamato!"));
}