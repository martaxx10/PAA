
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.h"
#include "Unit.generated.h"

// Enum per distinguere tra Brawler e Sniper
UENUM(BlueprintType)
enum class EUnitType: uint8
{
	Brawler UMETA(DisplayName = "Brawler"),
	Sniper UMETA(DisplayName = "Sniper")
};


UCLASS()
class PAA_API AUnit : public AActor
{
	GENERATED_BODY()
	
public:	
	AUnit();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// Tipo dell'unità (Brawler o Sniper)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Properties")
	EUnitType UnitClassType;

	// Proprietà dell'umanità
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Properties")
	int MovementRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Properties")
	int AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Properties")
	int MinDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Properties")
	int MaxDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Properties")
	int Health;
	
	// Indica se un'unità ha già agito
	UPROPERTY(BlueprintReadWrite)
	bool bHasActedThisTurn = false;

	void InitializeUnit(EUnitType Type);

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComponent;

	// Materiali per le unità
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* PlayerBrawlerMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* PlayerSniperMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* AIBrawlerMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* AISniperMaterial;

	void SetUnitMaterial(EUnitType UnitType, bool bIsPlayer);
	
	UPROPERTY()
	class ATile* CurrentTile;

	void SetCurrentTile(ATile* Tile);

	UFUNCTION(BlueprintCallable)
	bool IsInAttackRange(AUnit* TargetUnit);

	UFUNCTION(BlueprintCallable)
	int CalculateDamage();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Unit")
	EUnitType UnitType;
	
};
