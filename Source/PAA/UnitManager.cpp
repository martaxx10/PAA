#include "UnitManager.h"
#include "UnitActionWidget.h"



AUnitManager::AUnitManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Spawn dell'unità del player
void AUnitManager::SpawnPlayerUnit(AGrid* Grid, EUnitType UnitType, FIntPoint GridPosition)
{
    if (!Grid || Grid->TileGrid.Num() == 0 || 
        GridPosition.X < 0 || GridPosition.Y < 0 || 
        GridPosition.X >= Grid->GridSize || GridPosition.Y >= Grid->GridSize)
    {
        UE_LOG(LogTemp, Warning, TEXT(" Posizione non valida per spawnare il player."));
        return;
    }

    ATile* TargetTile = Grid->TileGrid[GridPosition.X][GridPosition.Y];
    if (!TargetTile || TargetTile->bIsObstacle)
    {
        UE_LOG(LogTemp, Warning, TEXT(" Tentativo di spawnare su un ostacolo!"));
        return;
    }

    FVector SpawnLocation = TargetTile->GetActorLocation() + FVector(0, 0, 50.0f);
    AUnit* NewUnit = Grid->GetWorld()->SpawnActor<AUnit>(AUnit::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    
    if (!NewUnit)
    {
        UE_LOG(LogTemp, Error, TEXT(" Spawn fallito."));
        return;
    }

    NewUnit->InitializeUnit(UnitType);
    NewUnit->SetUnitMaterial(UnitType, true);
    NewUnit->SetCurrentTile(TargetTile);
    Grid->PlayerUnits.Add(NewUnit);

    
    if (Grid->SelectedTile)
    {
        Grid->SelectedTile->SetHighlight(false);
    }
    Grid->SelectedTile = nullptr;
    Grid->SelectedUnit = nullptr;

    TargetTile->SetHighlight(false);  
    Grid->LastSpawnedTile = TargetTile;
    Grid->bJustSpawnedPlayerUnit = true;

    
    UE_LOG(LogTemp, Warning, TEXT(" Unità %s spawnata su [%s]"),
        (UnitType == EUnitType::Brawler ? TEXT("Brawler") : TEXT("Sniper")),
        *TargetTile->GetTileCoordinateLabel());

    Grid->PlayerUnitsToSpawn--;

   
    if (Grid->AIUnitsToSpawn > 0)
    {
        Grid->UnitManager->SpawnNextAIUnit(Grid);
        Grid->AIUnitsToSpawn--;
    }
    
    
    Grid->bIsPlayerTurn = false;
    if (Grid->PlayerUnitsToSpawn > 0 && Grid->UnitWidget)
    {
        Grid->UnitWidget->bIsSelectingTile = false;
        Grid->bIsPlayerTurn = true;
        UE_LOG(LogTemp, Warning, TEXT(" Tocca ancora al player."));
    }
    

    Grid->LastSpawnedTile = nullptr;
    
}



void AUnitManager::SpawnNextAIUnit(AGrid* Grid)
{
    if (!Grid || AIUnitsToSpawn.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("️ Nessuna unità AI da spawnare o Grid non valida."));
        return;
    }

    int32 Index = FMath::RandRange(0, AIUnitsToSpawn.Num() - 1);
    EUnitType UnitType = AIUnitsToSpawn[Index];
    AIUnitsToSpawn.RemoveAt(Index);

    TArray<FIntPoint> AvailableCells;
    for (int32 X = 0; X < Grid->GridSize; X++)
    {
        for (int32 Y = Grid->GridSize / 2; Y < Grid->GridSize; Y++)
        {
            if (!Grid->TileGrid[X][Y]->bIsObstacle)
            {
                AvailableCells.Add(FIntPoint(X, Y));
            }
        }
    }

    if (AvailableCells.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT(" Nessuna cella disponibile per lo spawn dell'AI!"));
        return;
    }

    FIntPoint ChosenPos = AvailableCells[FMath::RandRange(0, AvailableCells.Num() - 1)];
    ATile* TargetTile = Grid->TileGrid[ChosenPos.X][ChosenPos.Y];

    FVector SpawnLocation = TargetTile->GetActorLocation() + FVector(0, 0, 50);
    AUnit* NewUnit = Grid->GetWorld()->SpawnActor<AUnit>(AUnit::StaticClass(), SpawnLocation, FRotator::ZeroRotator);

    if (NewUnit)
    {
        NewUnit->InitializeUnit(UnitType);
        NewUnit->SetUnitMaterial(UnitType, false);
        NewUnit->SetCurrentTile(TargetTile); 

        
        Grid->AIUnits.Add(NewUnit);

       UE_LOG(LogTemp, Warning, TEXT(" Unità AI %s spawnata in [%s]"),
			(UnitType == EUnitType::Brawler) ? TEXT("Brawler") : TEXT("Sniper"),
			*Grid->TileGrid[ChosenPos.X][ChosenPos.Y]->GetTileCoordinateLabel());

       
    }
}
