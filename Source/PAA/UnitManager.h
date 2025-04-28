#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Unit.h"
#include "Grid.h"
#include "Tile.h"
#include "UnitManager.generated.h"

class AGrid;

UCLASS()
class PAA_API AUnitManager : public AActor
{
	GENERATED_BODY()
	
public:
	AUnitManager();

	TArray<EUnitType> AIUnitsToSpawn;

	// Spawna un'unità in una posizione scelta dal player
	void SpawnPlayerUnit(AGrid* Grid, EUnitType UnitType, FIntPoint GridPosition);

	void SpawnNextAIUnit(AGrid* Grid);


};
