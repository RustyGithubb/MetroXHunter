/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Debug/TickDebugger.h"
#include "AITargetComponent.generated.h"

class UAIAttackerComponent;

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
struct FAIReserverArray
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TArray<UAIAttackerComponent*> Data {};
};

//UINTERFACE( Blueprintable )
//class UAIGroupPlaceReserver : public UInterface
//{
//	GENERATED_BODY()
//};
//
//class METROXHUNTER_API IAIGroupPlaceReserver
//{
//	GENERATED_BODY()
//
//public:
//	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "AIGroupPlaceReserver" )
//};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UAITargetComponent : public UActorComponent, public ITickDebugger
{
	GENERATED_BODY()

public:
	UAITargetComponent();

	virtual void BeginPlay() override;

	void TickDebug_Implementation( float DeltaTime, FString& OutDebugText ) override;

	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens", meta = ( ReturnDisplayName = "bSuccess" ) )
	bool ReserveTokens( UAIAttackerComponent* Reserver, int32 Tokens );
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens", meta = ( ReturnDisplayName = "bSuccess" ) )
	bool FreeTokens( UAIAttackerComponent* Reserver, int32 Tokens = 0 );
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	void ClearTokens();
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	void SetTokenCooldown( float Seconds );

	UFUNCTION( BlueprintPure, Category = "AITarget|Tokens" )
	int32 GetReservedTokens( UAIAttackerComponent* Reserver ) const;
	UFUNCTION( BlueprintPure, Category = "AITarget|Tokens" )
	int32 GetRemainingTokens() const;
	UFUNCTION( BlueprintPure, Category = "AITarget|Tokens" )
	float GetTokenCooldown() const;

	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces", meta = ( ReturnDisplayName = "bSuccess" ) )
	bool ReserveGroupPlace( UAIAttackerComponent* Reserver, int32& GroupIndex );
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces" )
	void MoveGroupPlace( UAIAttackerComponent* Reserver, int32 NewGroupIndex );
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces", meta = ( ReturnDisplayName = "bSuccess" ) )
	bool FreeGroupPlace( UAIAttackerComponent* Reserver );

	UFUNCTION( BlueprintPure, Category = "AITarget|GroupPlaces", meta = ( ReturnDisplayName = "GroupSettings" ) )
	const FAITargetGroupSettings& GetGroupSettings( int32 GroupIndex ) const;
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces", meta = ( ReturnDisplayName = "GroupIndex" ) )
	int32 GetReservedGroupPlace( UAIAttackerComponent* Reserver ) const;
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces", meta = ( ReturnDisplayName = "RemainingPlaces" ) )
	int32 GetRemainingGroupPlaces( int32 GroupIndex ) const;
	UFUNCTION( BlueprintCallable, Category = "AITarget|GroupPlaces" )
	TMap<int32, FAIReserverArray> GetActorsByGroupPlaces() const;

	UFUNCTION( BlueprintCallable, Category = "AITarget|Attackers" )
	void DeclareAttacker( UAIAttackerComponent* Attacker );
	UFUNCTION( BlueprintCallable, Category = "AITarget|Attackers" )
	void RetireAttacker( UAIAttackerComponent* Attacker );
	UFUNCTION( BlueprintPure, Category = "AITarget|Attackers", meta = ( ReturnDisplayName = "Attackers" ) )
	int32 GetAttackersCount() const;

	UFUNCTION( BlueprintCallable, Category = "AITarget" )
	void FreeReservations( UAIAttackerComponent* Reserver );

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAttackersUpdate, UAITargetComponent*, AITargetComponent );
	UPROPERTY( BlueprintAssignable, Category = "AITarget" )
	FOnAttackersUpdate OnAttackersUpdate {};

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AITarget" )
	TArray<FAITargetGroupSettings> GroupsSettings {};

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AITarget" )
	int32 MaxTokens = 1;

private:
	float EndTokenCooldownTime = 0.0f;

	TMap<UAIAttackerComponent*, int32> ReservedTokens {};
	TMap<UAIAttackerComponent*, int32> ReservedGroupPlaces {};
	TSet<UAIAttackerComponent*> DeclaredAttackers {};
};
