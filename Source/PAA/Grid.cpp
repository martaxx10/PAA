#include "Grid.h"
#include "Tile.h"
#include "UnitManager.h"
#include "UnitActionWidget.h"
#include "UCoinTossWidget.h"
#include "Unit.h"
#include "Engine/World.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/Button.h"
#include "UObject/ConstructorHelpers.h"


// Costruttore
AGrid::AGrid()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	// Creiamo la GridMesh con un Instanced Static Mesh
	GridMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridMesh"));
	GridMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (MeshAsset.Succeeded())
		GridMesh->SetStaticMesh(MeshAsset.Object);

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Game/Materials/M_GridMaterial"));
	if (MaterialAsset.Succeeded())
		GridMesh->SetMaterial(0, MaterialAsset.Object);
		DefaultMaterial = MaterialAsset.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial> MountainAsset(TEXT("/Game/Materials/M_MountainMaterial"));
	if (MountainAsset.Succeeded())
		ObstacleMaterial_Mountain = MountainAsset.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial> Tree1Asset(TEXT("/Game/Materials/M_Tree1Material"));
	if (Tree1Asset.Succeeded())
		ObstacleMaterial_Tree1 = Tree1Asset.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial> Tree2Asset(TEXT("/Game/Materials/M_Tree2Material"));
	if (Tree2Asset.Succeeded())
		ObstacleMaterial_Tree2 = Tree2Asset.Object;

	UnitWidget = nullptr;  

	
}
	

void AGrid::BeginPlay()
	{
	
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("AGrid: BeginPlay avviato!"));

	FVector WorldGridCenter = FVector(0.0f, 0.0f, 0.0f);  // Punto centrale della scena
	SetActorLocation(WorldGridCenter);  // Imposta l'attore nel centro
	
	GenerateGrid();
	

	float ObstaclePercentage = 0.5f; // Percentuale degli ostacoli 
	GenerateObstacles(ObstaclePercentage); 

	UnitManager = GetWorld()->SpawnActor<AUnitManager>();

	SetupCamera();

	if (!UnitWidget)
	{
		if (!UnitActionWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT(" UnitActionWidgetClass non assegnato!"));
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT(" Creo UnitWidget da classe: %s"), *GetNameSafe(UnitActionWidgetClass));
		
		UnitWidget = CreateWidget<UUnitActionWidget>(GetWorld(), UnitActionWidgetClass);
		UnitWidget->GridRef = this;
		UE_LOG(LogTemp, Warning, TEXT(" UnitWidget creato una sola volta: %p"), UnitWidget);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(" UnitWidget già esistente, non lo ricreo!"));
	}

	if (UnitWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT(" UnitActionWidget creato: %p"), UnitWidget);

		UnitWidget->AddToViewport();
		UE_LOG(LogTemp, Warning, TEXT(" UnitWidget aggiunto alla viewport!"));
		

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" UnitWidget è NULL dopo CreateWidget!"));
	}

	if (UCoinTossWidgetClass)
	{
		UUCoinTossWidget* CoinWidget = CreateWidget<UUCoinTossWidget>(GetWorld(), UCoinTossWidgetClass);
		if (CoinWidget)
		{
			CoinWidget->GridRef = this; 
			CoinWidget->AddToViewport();
		}
	}

	
	// Abilita click del mouse
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->bEnableMouseOverEvents = true;
		PlayerController->bEnableClickEvents = true;
		UE_LOG(LogTemp, Warning, TEXT("✅ PlayerController: Click events abilitati!"));
	}

	UnitManager = GetWorld()->SpawnActor<AUnitManager>();

	// Inizializza le unità AI da spawnare (una Brawler e una Sniper)
	if (UnitManager)
	{
		UnitManager->AIUnitsToSpawn = { EUnitType::Brawler, EUnitType::Sniper };
	}
	
}

// Generazione della griglia
void AGrid::GenerateGrid()
{
	if (!GetWorld() || !GridMesh) return;

	TileGrid.SetNum(GridSize);
	FVector GridOrigin = FVector(0.0f, 0.0f, 0.0f);

	for (int32 X = 0; X < GridSize; X++)
	{
		TileGrid[X].SetNum(GridSize);
		for (int32 Y = 0; Y < GridSize; Y++)
		{
			FVector Location = GridOrigin + FVector(X * TileSize, Y * TileSize, 10.0f);
			FTransform TileTransform(FRotator(0, 0, 0), Location, FVector(1.0f, 1.0f, 1.0f));
			ATile* NewTile = GetWorld()->SpawnActor<ATile>(ATile::StaticClass(), TileTransform);
			NewTile->GridPosition = FIntPoint(X, Y);
			TileGrid[X][Y] = NewTile;
			GridMesh->AddInstance(TileTransform);
			
		}
	}
}



FIntPoint AGrid::WorldToGrid(const FVector& Location) const
{
	int32 X = FMath::RoundToInt(Location.X / TileSize);
	int32 Y = FMath::RoundToInt(Location.Y / TileSize);
	return FIntPoint(X, Y);
}

// Generazione ostacoli 
void AGrid::GenerateObstacles(float ObstaclePercentage)
{
    if (TileGrid.Num() == 0 || GridSize < 3) return;

    const int32 TotalCells = GridSize * GridSize;
    const int32 ObstacleCount = FMath::RoundToInt(TotalCells * ObstaclePercentage);
	

    // Reset: tutte le celle sono inizialmente libere
    for (int32 X = 0; X < GridSize; X++)
        for (int32 Y = 0; Y < GridSize; Y++)
            TileGrid[X][Y]->SetObstacle(false, nullptr);

    // Creazione della lista di tutte le celle disponibili
    TArray<FIntPoint> AvailableCells;
    for (int32 X = 0; X < GridSize; X++)
        for (int32 Y = 0; Y < GridSize; Y++)
            AvailableCells.Add(FIntPoint(X, Y));

    // Mescola casualmente le celle per posizionare gli ostacoli
    Algo::RandomShuffle(AvailableCells);

    int32 PlacedObstacles = 0;
    for (FIntPoint CellPos : AvailableCells)
    {
        if (PlacedObstacles >= ObstacleCount) break;

        ATile* Cell = TileGrid[CellPos.X][CellPos.Y];
        if (Cell && !Cell->bIsObstacle)
        {
            // Imposta come ostacolo temporaneo
            Cell->SetObstacle(true, ObstacleMaterial_Mountain);
            PlacedObstacles++;

            // Controlla se la griglia è ancora connessa
            if (!IsGridConnected())
            {
                // Se la griglia non è connessa, rimuovi l'ostacolo
                Cell->SetObstacle(false, nullptr);
                PlacedObstacles--;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Ostacoli finali posizionati: %d/%d (%.1f%%)"), 
        PlacedObstacles, TotalCells, (float)PlacedObstacles / TotalCells * 100);
}

// Controlla che la griglia sia connessa e che ci siano sempre percorsi liberi e celle non isolate
bool AGrid::IsGridConnected()
{
    TSet<FIntPoint> Visited;
    TQueue<FIntPoint> Queue;

    // Trova la prima cella libera
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            if (!TileGrid[X][Y]->bIsObstacle)
            {
                Queue.Enqueue(FIntPoint(X, Y));
                Visited.Add(FIntPoint(X, Y));
                break;
            }
        }
        if (!Queue.IsEmpty()) break;
    }

    // BFS per verificare la connettività
    const TArray<FIntPoint> Directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
    while (!Queue.IsEmpty())
    {
        FIntPoint Current;
        Queue.Dequeue(Current);

        for (const FIntPoint& Dir : Directions)
        {
            FIntPoint Neighbor = Current + Dir;
            if (Visited.Contains(Neighbor)) continue;

            if (Neighbor.X >= 0 && Neighbor.X < GridSize && 
                Neighbor.Y >= 0 && Neighbor.Y < GridSize && 
                !TileGrid[Neighbor.X][Neighbor.Y]->bIsObstacle)
            {
                Queue.Enqueue(Neighbor);
                Visited.Add(Neighbor);
            }
        }
    }

    // Conta le celle libere
    int32 FreeCells = 0;
    for (int32 X = 0; X < GridSize; X++)
        for (int32 Y = 0; Y < GridSize; Y++)
            if (!TileGrid[X][Y]->bIsObstacle) FreeCells++;

    return (Visited.Num() == FreeCells);
}

void AGrid::SetSelectedUnit(EUnitType UnitType)
{
	SelectedUnitType = UnitType;
	bIsUnitSelected = true;
    
	UE_LOG(LogTemp, Warning, TEXT("Unità selezionata: %s"), 
		(UnitType == EUnitType::Brawler) ? TEXT("Brawler") : TEXT("Sniper"));
}


FIntPoint AGrid::GetTilePosition(ATile* Tile) const
{
	for (int32 X = 0; X < GridSize; X++)
	{
		for (int32 Y = 0; Y < GridSize; Y++)
		{
			if (TileGrid[X][Y] == Tile)
			{
				return FIntPoint(X, Y);
			}
		}
	}
	return FIntPoint(-1, -1); // Valore di default se la tile non è trovata
}


void AGrid::ShowUnitSelectionUI()
{
	if (UnitWidget)
	{
		UnitWidget->AddToViewport();
	}
}

bool AGrid::IsValidTileCoord(int32 X, int32 Y) const
{
	return X >= 0 && X < GridSize && Y >= 0 && Y < GridSize;
}

FString AGrid::GetUnitTypeCode(AUnit* Unit) const
{
	if (!Unit) return TEXT("?");

	switch (Unit->UnitType)
	{
	case EUnitType::Brawler:
		return TEXT("B");
	case EUnitType::Sniper:
		return TEXT("S");
	default:
		return TEXT("?");
	}
}


void AGrid::AddMoveToHistory(const FString& Entry)
{
	if (!UnitWidget || !UnitWidget->SB_History || Entry.IsEmpty()) return;

	UTextBlock* EntryText = NewObject<UTextBlock>(UnitWidget, UTextBlock::StaticClass());
	if (EntryText)
	{
		EntryText->SetText(FText::FromString(Entry));
		EntryText->Font.Size = 20;
		EntryText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

		UnitWidget->SB_History->AddChild(EntryText);
	}
}

// Mostra il range di movimento dell'unità selezionata
void AGrid::ShowMovementRangeForSelectedUnit()
{
	if (!SelectedUnit || !SelectedUnit->CurrentTile) return;

	UE_LOG(LogTemp, Warning, TEXT(" BFS per range movimento di [%s]"), *SelectedUnit->GetName());

	// Reset di ogni highlight precedente
	for (int32 X = 0; X < GridSize; ++X)
	{
		for (int32 Y = 0; Y < GridSize; ++Y)
		{
			if (TileGrid[X][Y])
			{
				TileGrid[X][Y]->SetHighlight(false);
				TileGrid[X][Y]->SetAttackHighlight(false);  // reset anche attacco
			}
		}
	}

	// BFS queue
	TQueue<ATile*> Queue;
	TMap<ATile*, int32> Distance;
    
	ATile* StartTile = SelectedUnit->CurrentTile;
	Queue.Enqueue(StartTile);
	Distance.Add(StartTile, 0);

	while (!Queue.IsEmpty())
	{
		ATile* CurrentTile;
		Queue.Dequeue(CurrentTile);

		int32 CurrentDistance = Distance[CurrentTile];

		if (CurrentDistance > SelectedUnit->MovementRange)
			continue;

		// Evidenzia la cella corrente
		CurrentTile->SetHighlight(true);

		// Esplora le direzioni (su, giù, sinistra, destra)
		const TArray<FIntPoint> Directions = {
			FIntPoint(1, 0),
			FIntPoint(-1, 0),
			FIntPoint(0, 1),
			FIntPoint(0, -1)
		};

		for (const FIntPoint& Dir : Directions)
		{
			int32 NextX = CurrentTile->GridPosition.X + Dir.X;
			int32 NextY = CurrentTile->GridPosition.Y + Dir.Y;

			if (IsValidTileCoord(NextX, NextY))
			{
				ATile* Neighbor = TileGrid[NextX][NextY];

				if (Neighbor && !Neighbor->bIsObstacle && !Distance.Contains(Neighbor))
				{
					Distance.Add(Neighbor, CurrentDistance + 1);
					Queue.Enqueue(Neighbor);
				}
			}
		}
	}

	// Evidenzia le tile con nemici attaccabili
	for (AUnit* Enemy : AIUnits)
	{
		if (Enemy && Enemy->CurrentTile && SelectedUnit->IsInAttackRange(Enemy))
		{
			Enemy->CurrentTile->SetAttackHighlight(true);
			UE_LOG(LogTemp, Warning, TEXT(" [%s] è nel range di attacco di [%s]"), *Enemy->GetName(), *SelectedUnit->GetName());
		}
	}
	
}

// Mostra l'unità attaccabile
void AGrid::ShowAttackableUnitsForSelectedUnit()
{
	if (!SelectedUnit || !SelectedUnit->CurrentTile) return;

	for (AUnit* Enemy : AIUnits) // Usa PlayerUnits se sei l'AI
	{
		if (Enemy && Enemy->CurrentTile && SelectedUnit->IsInAttackRange(Enemy))
		{
			Enemy->CurrentTile->SetAttackHighlight(true);
			UE_LOG(LogTemp, Warning, TEXT("🎯 [%s] è nel range di attacco di [%s]"),
				*Enemy->GetName(), *SelectedUnit->GetName());
		}
	}
}


void AGrid::ClearMovementRange()
{
	for (int32 X = 0; X < GridSize; ++X)
	{
		for (int32 Y = 0; Y < GridSize; ++Y)
		{
			if (TileGrid[X][Y])
			{
				TileGrid[X][Y]->SetHighlight(false);
			}
		}
	}
}

// Trova il percorso tra due celle usando la BFS
TArray<ATile*> AGrid::FindPathBFS(ATile* StartTile, ATile* EndTile)
{
	TMap<ATile*, ATile*> CameFrom;
	TQueue<ATile*> Queue;
	TSet<ATile*> Visited;

	Queue.Enqueue(StartTile);
	Visited.Add(StartTile);
	CameFrom.Add(StartTile, nullptr);

	const TArray<FIntPoint> Directions = {
		FIntPoint(1, 0), FIntPoint(-1, 0),
		FIntPoint(0, 1), FIntPoint(0, -1)
	};
	
	
	while (!Queue.IsEmpty())
	{
		ATile* Current;
		Queue.Dequeue(Current);

		if (Current == EndTile)
		{
			// Se la EndTile è occupata da un'altra unità, non procedere
			bool bOccupied = false;
			for (AUnit* Unit : PlayerUnits)
			{
				if (Unit && Unit->CurrentTile == EndTile)
				{
					bOccupied = true;
					break;
				}
			}
			for (AUnit* Unit : AIUnits)
			{
				if (Unit && Unit->CurrentTile == EndTile)
				{
					bOccupied = true;
					break;
				}
			}
			
			if (bOccupied)
			{
				return {}; 
			}

			
			TArray<ATile*> Path;
			for (ATile* Tile = EndTile; Tile != nullptr; Tile = CameFrom[Tile])
			{
				Path.Insert(Tile, 0);
			}
			return Path;
		}


		for (const FIntPoint& Dir : Directions)
		{
			int32 NX = Current->GridPosition.X + Dir.X;
			int32 NY = Current->GridPosition.Y + Dir.Y;

			if (IsValidTileCoord(NX, NY))
			{
				ATile* Neighbor = TileGrid[NX][NY];
				if (Neighbor && !Neighbor->bIsObstacle && !Visited.Contains(Neighbor))
				{
					bool bOccupied = false;
					for (AUnit* Unit : PlayerUnits)
					{
						if (Unit && Unit->CurrentTile == Neighbor && Neighbor != StartTile)
						{
							bOccupied = true;
							break;
						}
					}
					for (AUnit* Unit : AIUnits)
					{
						if (Unit && Unit->CurrentTile == Neighbor && Neighbor != StartTile)
						{
							bOccupied = true;
							break;
						}
					}
					if (bOccupied) continue;

					Visited.Add(Neighbor);
					CameFrom.Add(Neighbor, Current);
					Queue.Enqueue(Neighbor);
				}
			}
		}
	}

	return {}; 
}

// Tentativo di attacco dopo il movimento
bool AGrid::TryAttackAfterMovement(AUnit* Attacker)
{
	for (AUnit* PlayerUnit : PlayerUnits)
	{
		if (!PlayerUnit || PlayerUnit == Attacker) continue; // evita autocolpo

		if (Attacker->IsInAttackRange(PlayerUnit))
		{
			if (PlayerUnit->CurrentTile)
			{
				PlayerUnit->CurrentTile->SetAttackHighlight(true);
				UE_LOG(LogTemp, Warning, TEXT(" AI evidenzia [%s] come bersaglio"), *PlayerUnit->GetName());
			}

			int Damage = Attacker->CalculateDamage();
			
			PlayerUnit->Health -= Damage;
			
			if (UnitWidget)
			{
				UnitWidget->UpdateHealthBar(PlayerUnit);
			}

			
			UE_LOG(LogTemp, Warning, TEXT(" AI ATTACCA da [%s] a [%s]! Danno: %d"),
				*Attacker->CurrentTile->GetTileCoordinateLabel(),
				*PlayerUnit->CurrentTile->GetTileCoordinateLabel(),
				Damage);
				FString TeamCode = AIUnits.Contains(Attacker) ? "AI" : "HP";
				FString TypeCode = GetUnitTypeCode(Attacker);
				FString TargetCoord = PlayerUnit->CurrentTile->GetTileCoordinateLabel();
				FString AttackEntry = FString::Printf(TEXT("%s: %s %s %d"), *TeamCode, *TypeCode, *TargetCoord, Damage);
				AddMoveToHistory(AttackEntry);


			if (PlayerUnit->Health <= 0)
			{
				PlayerUnits.Remove(PlayerUnit);
				PlayerUnit->Destroy();
				UE_LOG(LogTemp, Warning, TEXT(" Unità del Player eliminata!"));
			}

			FTimerHandle CleanupHighlightTimer;
			GetWorld()->GetTimerManager().SetTimer(CleanupHighlightTimer, [this]()
			{
				ATile::ClearAllTileHighlights(TileGrid);
			}, 0.4f, false);

			CurrentAIIndex++;
			return true;
		}
	}
	return false; 
}

void AGrid::HandleSkipAttack()
{
	if (!SelectedUnit) return;

	SelectedUnit->bHasActedThisTurn = true; 
	SelectedUnit = nullptr;
	UnitWidget->bActionChosenForUnit = false;
	SelectedTile = nullptr;
	bIsInActionPhase = false;
	bIsAttackMode = false;
	bIsMoveMode = false;

	if (UnitWidget)
	{
		UnitWidget->BT_SkipAttack->SetVisibility(ESlateVisibility::Collapsed);
		UnitWidget->bActionChosenForUnit = false;  
		UnitWidget->HideErrorMessage();  
	}

	bool bAllUnitsActed = true;
	for (AUnit* PlayerUnit : PlayerUnits)
	{
		if (PlayerUnit && !PlayerUnit->bHasActedThisTurn)
		{
			bAllUnitsActed = false;
			break;
		}
	}

	if (bAllUnitsActed)
	{
		bIsPlayerTurn = false;
		if (UnitWidget) UnitWidget->UpdateTurnText(false);
		
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]() {
			HandleAITurn();
		}, 0.5f, false);
	}
}

//Fa muovere l'unità passo passo 
void AGrid::MoveUnitAlongPath(AUnit* Unit, const TArray<ATile*>& Path)
{
	if (!Unit || Path.Num() == 0) return;
	
	ATile* StartTile = Unit->CurrentTile;
	FTimerHandle* MoveTimer = new FTimerHandle(); 
	int32 StepIndex = 0;

	
	FTimerDelegate Delegate;
	Delegate.BindLambda([this, Unit, Path, StepIndex, MoveTimer, StartTile]() mutable

	{
		if (!IsValid(Unit))
		{
			GetWorld()->GetTimerManager().ClearTimer(*MoveTimer);
			delete MoveTimer;
			UE_LOG(LogTemp, Error, TEXT(" Crash evitato: Unit è nullptr in MoveUnitAlongPath!"));
			return;
		}
		
		if (StepIndex >= Path.Num())
		{
			GetWorld()->GetTimerManager().ClearTimer(*MoveTimer);
			delete MoveTimer; // libera la memoria del timer

			ATile* FinalTile = Path.Last();

			if (FinalTile)
			{
				Unit->SetActorLocation(FinalTile->GetActorLocation() + FVector(0, 0, 50));
				Unit->SetCurrentTile(FinalTile);
				Unit->bHasActedThisTurn = true;
				UE_LOG(LogTemp, Warning, TEXT(" Unità arrivata su [%s]"), *FinalTile->GetTileCoordinateLabel());

				// Log movimento nella cronologia
				FString TeamCode = PlayerUnits.Contains(Unit) ? TEXT("HP") : TEXT("AI");
				FString TypeCode = GetUnitTypeCode(Unit);
				FString FromCoord = StartTile ? StartTile->GetTileCoordinateLabel() : TEXT("???");
				FString ToCoord = FinalTile ? FinalTile->GetTileCoordinateLabel() : TEXT("???");
				FString MoveEntry = FString::Printf(TEXT("%s: %s %s -> %s"), *TeamCode, *TypeCode, *FromCoord, *ToCoord);
				AddMoveToHistory(MoveEntry);
	
			}

			// Rimuove tutti gli highlight
			ATile::ClearAllTileHighlights(TileGrid);
			
			
			bool bDidAttack = false;
			if (AIUnits.Contains(Unit))
			{
				bDidAttack = TryAttackAfterMovement(Unit);
			}

			
			// Se è il turno del Player
			if (PlayerUnits.Contains(Unit))
			{
				// Controlla se ci sono nemici nel range di attacco
				bool bHasTargets = false;
				for (AUnit* Enemy : AIUnits)
				{
					if (Enemy && Unit->IsInAttackRange(Enemy))
					{
						bHasTargets = true;
						break;
					}
				}

				if (bHasTargets)
				{
					bIsInActionPhase = true;
					bIsAttackMode = false;

					SelectedUnit = Unit;
					SelectedTile = Unit->CurrentTile;
					SelectedTile->SetHighlight(true);

					ShowAttackableUnitsForSelectedUnit();
					
					UE_LOG(LogTemp, Warning, TEXT(" Mostro i bersagli attaccabili dopo il movimento di [%s]"), *Unit->GetName());

					if (UnitWidget && UnitWidget->BT_SkipAttack)
					{
						UnitWidget->BT_SkipAttack->SetVisibility(ESlateVisibility::Visible);
					}
					
				}
				else
				{
					// Nessun bersaglio → gestisco il turno come concluso per questa unità
					SelectedUnit = nullptr;
					SelectedTile = nullptr;
					bIsInActionPhase = false;
					bIsAttackMode = false;
					bIsMoveMode = false;

					UE_LOG(LogTemp, Warning, TEXT(" Nessun bersaglio nel range dopo il movimento di [%s]"), *Unit->GetName());

					bool bAllUnitsActed = true;
					for (AUnit* PlayerUnit : PlayerUnits)
					{
						if (PlayerUnit && !PlayerUnit->bHasActedThisTurn)
						{
							bAllUnitsActed = false;
							break;
						}
					}

					if (bAllUnitsActed)
					{
						bIsPlayerTurn = false;
						CurrentAIIndex = 0; 

						if (UnitWidget)
							UnitWidget->UpdateTurnText(false);

						FTimerHandle TimerHandle;
						GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]() {
							HandleAITurn();
						}, 0.5f, false);
					}
				}
				UnitWidget->bActionChosenForUnit = false;

			}
			else
			{
				
				Unit->bHasActedThisTurn = true;

				// Se tutte le AI hanno agito, passa al Player
				bool bAllAIActed = true;
				for (AUnit* AIUnit : AIUnits)
				{
					if (AIUnit && !AIUnit->bHasActedThisTurn)
					{
						bAllAIActed = false;
						break;
					}
				}

				if (bAllAIActed)
				{
					// Reset Player per nuovo turno
					for (AUnit* PlayerUnit : PlayerUnits)
						if (PlayerUnit) PlayerUnit->bHasActedThisTurn = false;

					bIsPlayerTurn = true;
					CurrentAIIndex = 0;

					if (UnitWidget)
					{
						UnitWidget->bActionChosenForUnit = false;
						UnitWidget->UpdateTurnText(true);
						UnitWidget->HideErrorMessage(); // se presente
					}

					ShowUnitSelectionUI();
					UE_LOG(LogTemp, Warning, TEXT(" Fine turno AI → Tocca al Player."));
				}
				else
				{
					// Prosegui con la prossima unità AI anche se ha attaccato
					FTimerHandle TimerHandle;
					GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
					{
						HandleAITurn();
					}, 0.5f, false);

					UE_LOG(LogTemp, Warning, TEXT(" Continuo turno AI: passo alla prossima unità."));
				}

			}

			return;
		}

		ATile* StepTile = Path[StepIndex];
		if (StepTile)
		{
			FVector NewLocation = StepTile->GetActorLocation() + FVector(0, 0, 50);
			Unit->SetActorLocation(NewLocation);
			Unit->SetCurrentTile(StepTile);
			UE_LOG(LogTemp, Warning, TEXT("🚶‍♂️ Step %d → [%s]"), StepIndex + 1, *StepTile->GetTileCoordinateLabel());
		}

		StepIndex++;
	});

	// Passa il timer handle per riferimento
	GetWorld()->GetTimerManager().SetTimer(*MoveTimer, Delegate, 0.2f, true);
}

// Gestisce il turno dell'AI
void AGrid::HandleAITurn()
{
	UE_LOG(LogTemp, Warning, TEXT(" HandleAITurn() chiamata."));
	
	if (CurrentAIIndex == 0) 
	{
		for (AUnit* AIUnit : AIUnits)
		{
			if (AIUnit)
				AIUnit->bHasActedThisTurn = false;
		}
	}
	
	if (CurrentAIIndex >= AIUnits.Num())
	{
		// Reset Player per nuovo turno
		for (AUnit* PlayerUnit : PlayerUnits)
			if (PlayerUnit) PlayerUnit->bHasActedThisTurn = false;

		bIsPlayerTurn = true;
		CurrentAIIndex = 0;
		if (UnitWidget) UnitWidget->UpdateTurnText(true);
		ShowUnitSelectionUI();
		UE_LOG(LogTemp, Warning, TEXT(" Fine turno AI → Tocca al Player."));
		return;
	}

	AUnit* AIUnit = AIUnits[CurrentAIIndex];
	CurrentAIIndex++;

	if (UnitWidget) UnitWidget->bActionChosenForUnit = false;

	if (!AIUnit || AIUnit->bHasActedThisTurn || !AIUnit->CurrentTile)
	{
		// Vai direttamente alla prossima unità
		FTimerHandle NextAITimer;
		GetWorld()->GetTimerManager().SetTimer(NextAITimer, [this]() {
			HandleAITurn();
		}, 0.1f, false);
		return;
	}


	for (AUnit* PlayerTarget : PlayerUnits)
	{
		if (PlayerTarget && AIUnit->IsInAttackRange(PlayerTarget))
		{
			int Damage = AIUnit->CalculateDamage();
			PlayerTarget->Health -= Damage;

			// Controattacco
			if (AIUnit->UnitType == EUnitType::Sniper)
			{
				bool bShouldCounterAttack = false;

				if (PlayerTarget->UnitType == EUnitType::Sniper)
				{
					bShouldCounterAttack = true;
				}
				else if (PlayerTarget->UnitType == EUnitType::Brawler)
				{
					int32 Distance = FMath::Abs(AIUnit->CurrentTile->GridPosition.X - PlayerTarget->CurrentTile->GridPosition.X)
								   + FMath::Abs(AIUnit->CurrentTile->GridPosition.Y - PlayerTarget->CurrentTile->GridPosition.Y);

					if (Distance == 1)
					{
						bShouldCounterAttack = true;
					}
				}

				if (bShouldCounterAttack)
				{
					int32 CounterDamage = FMath::RandRange(1, 3);
					AIUnit->Health -= CounterDamage;

					if (UnitWidget)
					{
						UnitWidget->UpdateHealthBar(AIUnit);
					}

					UE_LOG(LogTemp, Warning, TEXT(" AI subisce contrattacco! Danno: %d"), CounterDamage);

					if (AIUnit->Health <= 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("️ Unità AI uccisa da contrattacco!"));
						AIUnits.Remove(AIUnit);
						AIUnit->Destroy();
						CheckGameOver();
						return;
					}
				}
			}

			if (UnitWidget) UnitWidget->UpdateHealthBar(PlayerTarget);

			UE_LOG(LogTemp, Warning, TEXT(" AI ATTACCA da [%s] a [%s]! Danno: %d"),
				*AIUnit->CurrentTile->GetTileCoordinateLabel(),
				*PlayerTarget->CurrentTile->GetTileCoordinateLabel(),
				Damage);

			if (PlayerTarget->CurrentTile)
				PlayerTarget->CurrentTile->SetAttackHighlight(true);

			FTimerHandle ClearHighlightHandle;
			GetWorld()->GetTimerManager().SetTimer(ClearHighlightHandle, [PlayerTarget]() {
				if (PlayerTarget && PlayerTarget->CurrentTile)
				{
					PlayerTarget->CurrentTile->SetAttackHighlight(false);
				}
			}, 0.4f, false); // Aspetta 0.6 secondi per dare un feedback visivo

			FString TeamCode = TEXT("AI");
			FString TypeCode = GetUnitTypeCode(AIUnit);
			FString TargetCoord = PlayerTarget->CurrentTile->GetTileCoordinateLabel();
			FString AttackEntry = FString::Printf(TEXT("%s: %s %s %d"), *TeamCode, *TypeCode, *TargetCoord, Damage);
			AddMoveToHistory(AttackEntry);

			if (PlayerTarget->Health <= 0)
			{
				PlayerUnits.Remove(PlayerTarget);
				PlayerTarget->Destroy();
				UE_LOG(LogTemp, Warning, TEXT(" Unità Player eliminata!"));

				CheckGameOver(); 
			}

			AIUnit->bHasActedThisTurn = true;

			bool bAllAIActed = true;
			for (AUnit* AIUnitCheck : AIUnits)
			{
				if (AIUnitCheck && !AIUnitCheck->bHasActedThisTurn)
				{
					bAllAIActed = false;
					break;
				}
			}

			if (bAllAIActed)
			{
				// Reset azioni Player
				for (AUnit* PlayerUnit : PlayerUnits)
				{
					if (PlayerUnit) PlayerUnit->bHasActedThisTurn = false;
				}

				// Passa al Player
				bIsPlayerTurn = true;
				CurrentAIIndex = 0;

				// Reset del flag che blocca le azioni nel widget
				if (UnitWidget)
				{
					UnitWidget->bActionChosenForUnit = false;
					UnitWidget->UpdateTurnText(true);
					UnitWidget->HideErrorMessage(); // se presente
				}

				ShowUnitSelectionUI();
				UE_LOG(LogTemp, Warning, TEXT(" Fine turno AI → Tocca al Player."));
			}

			else
			{
				FTimerHandle NextAITimer;
				GetWorld()->GetTimerManager().SetTimer(NextAITimer, [this]() {
					HandleAITurn();
				}, 0.5f, false);
			}


			return;
		}
	}

	// Movimento con attacco automatico
	TArray<ATile*> Reachable;
	TMap<ATile*, int32> Distance;
	TSet<ATile*> Visited;
	TQueue<ATile*> Queue;

	Queue.Enqueue(AIUnit->CurrentTile);
	Distance.Add(AIUnit->CurrentTile, 0);
	Visited.Add(AIUnit->CurrentTile);

	while (!Queue.IsEmpty())
	{
		ATile* Current;
		Queue.Dequeue(Current);
		int32 CurrDist = Distance[Current];
		if (CurrDist >= AIUnit->MovementRange) continue;

		const TArray<FIntPoint> Dirs = { {1,0}, {-1,0}, {0,1}, {0,-1} };
		for (const FIntPoint& Dir : Dirs)
		{
			int32 X = Current->GridPosition.X + Dir.X;
			int32 Y = Current->GridPosition.Y + Dir.Y;

			if (IsValidTileCoord(X, Y))
			{
				ATile* Neighbor = TileGrid[X][Y];
				if (Neighbor && !Neighbor->bIsObstacle && !Visited.Contains(Neighbor))
				{
					Queue.Enqueue(Neighbor);
					Distance.Add(Neighbor, CurrDist + 1);
					Visited.Add(Neighbor);
				}
			}
		}
	}

	for (ATile* Tile : Visited)
		if (Tile != AIUnit->CurrentTile)
			Reachable.Add(Tile);

	TArray<ATile*> Path;
	ATile* Destination = nullptr;

	int32 BestDistance = TNumericLimits<int32>::Max();
	for (ATile* Tile : Reachable)
	{
		TArray<ATile*> TempPath = FindPathBFS(AIUnit->CurrentTile, Tile);
		if (TempPath.Num() == 0) continue;

		for (AUnit* PlayerUnit : PlayerUnits)
		{
			if (!PlayerUnit || !PlayerUnit->CurrentTile) continue;
			int32 Dist = FMath::Abs(Tile->GridPosition.X - PlayerUnit->CurrentTile->GridPosition.X)
					   + FMath::Abs(Tile->GridPosition.Y - PlayerUnit->CurrentTile->GridPosition.Y);

			if (Dist < BestDistance)
			{
				BestDistance = Dist;
				Destination = Tile;
				Path = TempPath;
			}
		}
	}

	if (!Destination || Path.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT(" Nessun percorso valido per AI [%s]"), *AIUnit->GetName());
		AIUnit->bHasActedThisTurn = true;
		HandleAITurn();
		return;
	}

	// Highlight percorso
	for (ATile* Tile : Path)
	{
		if (Tile) Tile->SetHighlight(true);
	}

	UE_LOG(LogTemp, Warning, TEXT(" AI [%s] si muove da [%s] a [%s] (%d step)"),
		*AIUnit->GetName(),
		*AIUnit->CurrentTile->GetTileCoordinateLabel(),
		*Destination->GetTileCoordinateLabel(),
		Path.Num());

	MoveUnitAlongPath(AIUnit, Path);
}


// Verifica se qualcuno ha vinto
void AGrid::CheckGameOver()
{
	if (PlayerUnits.Num() == 0)
	{
		ShowGameOverMessage(TEXT(" AI won!"));
		return;
	}
	
	if (AIUnits.Num() == 0)
	{
		ShowGameOverMessage(TEXT(" You won!"));
		return;
	}
}

// Mostra il messaggio di vittoria
void AGrid::ShowGameOverMessage(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);

	if (UnitWidget && UnitWidget->TXT_Message)
	{
		UnitWidget->TXT_Message->SetText(FText::FromString(Message));
		UnitWidget->TXT_Message->SetVisibility(ESlateVisibility::Visible);
	}

	// Delay per mostrare il messaggio prima del restart
	FTimerHandle RestartHandle;
	GetWorld()->GetTimerManager().SetTimer(RestartHandle, []()
	{
		UGameplayStatics::OpenLevel(GWorld, FName(*UGameplayStatics::GetCurrentLevelName(GWorld)));
	}, 3.0f, false);
}


void AGrid::SetupCamera()
{
	if (!GetWorld()) return;

	GridCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass());
	FVector GridCenter = FVector((GridSize * TileSize) * 0.5f, (GridSize * TileSize) * 0.5f, 0);
	GridCamera->SetActorLocation(GridCenter + FVector(0, 0, GridSize * TileSize * 5.5f));
	GridCamera->SetActorRotation(FRotator(-90.0f, 0.0f, 0.0f));

	UCameraComponent* CameraComponent = GridCamera->GetCameraComponent();
	if (CameraComponent)
	{
		CameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
		CameraComponent->SetOrthoWidth(GridSize * TileSize * 2.0f);
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
		PlayerController->SetViewTarget(GridCamera);
}

void AGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}