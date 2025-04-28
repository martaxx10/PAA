#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.h"
class AUnitManager;
#include "Unit.h"
class UUnitActionWidget;
class UUCoinTossWidget;
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"
#include "Grid.generated.h"

UCLASS()
class PAA_API AGrid : public AActor
{
	GENERATED_BODY()
    
public: 
	AGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

	
public:
	
	virtual void Tick(float DeltaTime) override;

	// Matrice per tenere traccia delle celle nella griglia
	TArray<TArray<ATile*>> TileGrid;

	// Liste delle unità
	TArray<AUnit*> PlayerUnits;
	TArray<AUnit*> AIUnits;
    
	// Impostazioni griglia
	UPROPERTY(EditAnywhere, Category = "Grid Settings")
	int32 GridSize = 25;

	UPROPERTY(EditAnywhere, Category = "Grid Settings")
	float TileSize = 110.0f;

	UPROPERTY(EditAnywhere, Category = "Grid Settings")
	UInstancedStaticMeshComponent* GridMesh;

	UPROPERTY(EditAnywhere, Category = "Grid Camera")
	ACameraActor* GridCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid")
	int32 MinPathLength; // Numero minimo di celle libere

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUnitActionWidget> UnitActionWidgetClass;

	UPROPERTY()
	UUnitActionWidget* UnitWidget;

	// Classe del widget per lancio della moneta
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UUCoinTossWidget> UCoinTossWidgetClass;


	// Metodo per ottenere la posizione di una tile nella griglia
	FIntPoint GetTilePosition(ATile* Tile) const;
	
	AUnitManager* UnitManager;
	UUnitActionWidget* UnitActionWidgetWidget;

	// Funzioni principali
	void GenerateObstacles(float ObstaclePercentage);
	void GenerateGrid();
	void SetupCamera();
	bool IsGridConnected(); 
	
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetSelectedUnit(EUnitType UnitType);

	UPROPERTY()
	bool bAIUnitsSpawned = false;

	int32 PlayerUnitsToSpawn = 2;
	int32 AIUnitsToSpawn = 2;

	bool bIsPlayerTurn = true;

	int32 AIUnitsSpawned = 0;
	int32 PlayerUnitsSpawned = 0;
	
	UFUNCTION()
	void ShowUnitSelectionUI();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bGameStarted = false;

	UPROPERTY()
	AUnit* SelectedUnit = nullptr;

	UPROPERTY()
	ATile* SelectedTile = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* HighlightTileMaterial;

	FIntPoint WorldToGrid(const FVector& Location) const;

	bool bIsInUnitSelectionPhase = false; 

	UPROPERTY()
	ATile* LastSpawnedTile = nullptr;
	
	// Serve per ignorare il click subito dopo lo spawn
	bool bJustSpawnedPlayerUnit = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsInActionPhase = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMoveMode = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsAttackMode = false;

	void HandleSkipAttack();
	
	UFUNCTION()
	void ShowMovementRangeForSelectedUnit();
	
	void ShowAttackableUnitsForSelectedUnit();
	void ClearMovementRange();
	

	UFUNCTION()
	bool IsValidTileCoord(int32 X, int32 Y) const;

	TArray<ATile*> FindPathBFS(ATile* StartTile, ATile* EndTile);
	
	bool TryAttackAfterMovement(AUnit* Attacker);

	void MoveUnitAlongPath(AUnit* Unit, const TArray<ATile*>& Path);

	void HandleAITurn();
	
	FTimerHandle MovementTimerHandle;

	bool bDidPlayerWinToss = true;  
	bool bActionTurnPlayer = true;  
	bool bPlayerStartsFirst = true;
	
	UPROPERTY()
	int32 CurrentAIIndex = 0;
	
	UPROPERTY()
	TArray<AUnit*> UnitsActedThisTurn;

	UFUNCTION()
	void AddMoveToHistory(const FString& Entry);

	UFUNCTION()
	void CheckGameOver();

	UFUNCTION()
	void ShowGameOverMessage(const FString& Message);

	FString GetUnitTypeCode(AUnit* Unit) const;


private:

	
	// Set di posizioni degli ostacoli
	TSet<FIntPoint> ObstaclePositions;
	
	// Materiali
	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterial* DefaultMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* ObstacleMaterial_Mountain;

	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* ObstacleMaterial_Tree1;

	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* ObstacleMaterial_Tree2;

	EUnitType SelectedUnitType;
	bool bIsUnitSelected = false;

	
};
