/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITargetComponent.generated.h"

USTRUCT( BlueprintType )
struct FAITargetGroupSettings
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	int32 MaxPlaces = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FFloatRange PlaceDistance { 350.0f, 450.0f };
};

USTRUCT( BlueprintType )
struct FActorArray
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TArray<AActor*> Array {};
};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UAITargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAITargetComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent( 
		float DeltaTime,
		ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction
	) override;

	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	bool ReserveTokens( AActor* Reserver, int32 Tokens );
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	bool FreeTokens( AActor* Reserver, int32 Tokens = 0 );
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	void ClearTokens();
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	void SetTokenCooldown( float Seconds );

	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	int32 GetReservedTokens( AActor* Reserver ) const;
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	int32 GetRemainingTokens() const;
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	float GetTokenCooldown() const;

	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces" )
	bool ReserveGroupPlace( AActor* Reserver, int32& GroupIndex );
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces" )
	void MoveGroupPlace( AActor* Reserver, int32 NewGroupIndex );
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces" )
	bool FreeGroupPlace( AActor* Reserver );

	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces" )
	int32 GetReservedGroupPlace( AActor* Reserver ) const;
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces" )
	int32 GetRemainingGroupPlaces( int32 GroupIndex ) const;
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces" )
	TMap<int32, FActorArray> GetActorsByGroupPlaces() const;

	UFUNCTION( BlueprintCallable, Category = "AITarget" )
	void FreeReservations( AActor* Reserver );

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AITarget" )
	TArray<FAITargetGroupSettings> GroupsSettings {};

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AITarget" )
	int32 MaxTokens = 1;

private:
	TMap<AActor*, int32> ReservedTokens {};
	TMap<AActor*, int32> ReservedGroupPlaces {};

	float EndTokenCooldownTime = 0.0f;
};
