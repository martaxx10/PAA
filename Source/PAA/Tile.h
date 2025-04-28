#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Tile.generated.h"

UCLASS()
class PAA_API ATile : public AActor
{
	GENERATED_BODY()

public:    
	ATile();

protected:
	virtual void BeginPlay() override;
	

public:

	UPROPERTY(EditAnywhere, Category = "Tile Settings")
	bool bIsObstacle = false;

	UPROPERTY(EditAnywhere, Category = "Tile Settings")
	FIntPoint GridPosition; // Posizione della cella nella griglia

	
	void SetObstacle(bool bNewState, UMaterialInterface* ObstacleMat);
	
	UFUNCTION()
	void OnTileClicked(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);

	UFUNCTION()
	void SetHighlight(bool bEnable);

	// Cancella  tutti gli highlight
	static void ClearAllTileHighlights(const TArray<TArray<ATile*>>& TileGrid);

	// Mesh della tile
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	// Materiali 
	UPROPERTY(EditAnywhere)
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UMaterialInterface* HighlightedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	UMaterialInterface* AttackHighlightMaterial;
	
	UFUNCTION()
	void SetHighlighted(bool bHighlighted);

	UFUNCTION()
	void ShowMovementRange(AUnit* Unit, int MaxRange);

	UFUNCTION()
	void ClearHighlightIfNotSelected();

	
	bool bIsAttackHighlighted = false;
	
	// Restituisce il nome della cella in formato "A1", "B7", ecc. */
	FString GetTileCoordinateLabel() const;

	void SetAttackHighlight(bool bEnable);

private:
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* TileMesh;
	
	// Materiale per gli ostacoli (montagne, alberi)
	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* ObstacleMaterial_Mountain;

	UFUNCTION()
	void OnTileMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	
};
