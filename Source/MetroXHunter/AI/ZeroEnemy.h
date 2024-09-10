#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/ZeroEnemyData.h"
#include "ZeroEnemy.generated.h"

UCLASS()
class METROXHUNTER_API AZeroEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AZeroEnemy();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void OpenBulb( float OpenTime = 0.0f );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void CloseBulb();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void Stun( float StunTime = 0.0f );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void UnStun();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void MakePanic();
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void DestroyBodyPart( USceneComponent* BodyPart );

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void ApplyKnockback( const FVector& Direction, float Force );

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "ZeroEnemy" )
	bool IsBulbOpened() const { return bIsBulbOpened; }

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnStun );
	UPROPERTY( BlueprintAssignable, EditDefaultsOnly, Category = "ZeroEnemy" )
	FOnStun OnStun;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnUnStun );
	UPROPERTY( BlueprintAssignable, EditDefaultsOnly, Category = "ZeroEnemy" )
	FOnStun OnUnStun;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ZeroEnemy" )
	UStaticMeshComponent* ProtoMeshComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ZeroEnemy" )
	UStaticMeshComponent* BulbMeshComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ZeroEnemy" )
	UZeroEnemyData* Data = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ZeroEnemy" )
	int32 LeftBodyPartsCount = 0;

	//  TODO: Move to private
	UPROPERTY( BlueprintReadOnly, Category = "ZeroEnemy" )
	bool bIsAlive = true;

private:
	void GenerateBulb();
	void RetrieveReferences();

	void UpdateWalkSpeed();

private:
	bool bIsBulbOpened = false;
	bool bIsStun = false;

	FRotator StartStunRotation {};

	int32 StartBodyPartsCount = 0;

	FTimerHandle OpeningBulbTimerHandle {};
	FTimerHandle StunTimerHandle {};
};
