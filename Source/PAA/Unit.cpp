#include "Unit.h"


// Costruttore
AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Carica la mesh base
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (MeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshAsset.Object);
		MeshComponent->SetWorldScale3D(FVector(1.5f, 1.5f, 1.0f));
	}

	// Carica il materiale trasparente iniziale 
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TransparentMat(TEXT("/Game/Materials/M_Transparent.M_Transparent"));
	if (TransparentMat.Succeeded())
	{
		MeshComponent->SetMaterial(0, TransparentMat.Object);
	}

	// Carica i materiali per Player/AI
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerBrawlerMat(TEXT("/Game/Materials/Player_Brawler"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerSniperMat(TEXT("/Game/Materials/Player_Sniper"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIBrawlerMat(TEXT("/Game/Materials/AI_Brawler"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AISniperMat(TEXT("/Game/Materials/AI_Sniper"));

	if (PlayerBrawlerMat.Succeeded()) PlayerBrawlerMaterial = PlayerBrawlerMat.Object;
	if (PlayerSniperMat.Succeeded()) PlayerSniperMaterial = PlayerSniperMat.Object;
	if (AIBrawlerMat.Succeeded()) AIBrawlerMaterial = AIBrawlerMat.Object;
	if (AISniperMat.Succeeded()) AISniperMaterial = AISniperMat.Object;

	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	

}

void AUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUnit::SetUnitMaterial(EUnitType NewUnitType, bool bIsPlayer)
{
	UMaterialInterface* ChosenMaterial = nullptr;

	if (bIsPlayer)
	{
		ChosenMaterial = (NewUnitType == EUnitType::Brawler) ? PlayerBrawlerMaterial : PlayerSniperMaterial;
	}
	else
	{
		ChosenMaterial = (NewUnitType == EUnitType::Brawler) ? AIBrawlerMaterial : AISniperMaterial;
	}

	if (ChosenMaterial && MeshComponent)
	{
		MeshComponent->SetMaterial(0, ChosenMaterial);
		UE_LOG(LogTemp, Warning, TEXT("✅ Materiale applicato all'unità!"));
	}
}

void AUnit::SetCurrentTile(ATile* Tile)
{
	CurrentTile = Tile;
	UE_LOG(LogTemp, Warning, TEXT("📌 SetCurrentTile: %s assegnata alla unità [%s]"), *Tile->GetTileCoordinateLabel(), *GetName());
}

// Inizializza i valori in base al tipo di unità
void AUnit::InitializeUnit(EUnitType Type)
{
	UnitType = Type;
	UnitClassType = Type;
    
	if (Type == EUnitType::Brawler)
	{
		MovementRange = 6;
		AttackRange = 1;
		MinDamage = 1;
		MaxDamage = 6;
		Health = 40;
	}
	else if (Type == EUnitType::Sniper)
	{
		MovementRange = 3;
		AttackRange = 10;
		MinDamage = 4;
		MaxDamage = 8;
		Health = 20;
	}
}


bool AUnit::IsInAttackRange(AUnit* TargetUnit)
{
	if (!TargetUnit || !TargetUnit->CurrentTile || !CurrentTile)
		return false;

	int32 DX = FMath::Abs(CurrentTile->GridPosition.X - TargetUnit->CurrentTile->GridPosition.X);
	int32 DY = FMath::Abs(CurrentTile->GridPosition.Y - TargetUnit->CurrentTile->GridPosition.Y);

	return (DX + DY) <= AttackRange;
}

int AUnit::CalculateDamage()
{
	return FMath::RandRange(MinDamage, MaxDamage);
}

