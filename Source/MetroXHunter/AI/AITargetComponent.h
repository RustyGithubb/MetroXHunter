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

UENUM( BlueprintType )
enum class EAITokenSlot : uint8
{
	Melee,
	Distance,
	MAX			UMETA( Hidden ),
};

struct FAIReservedToken
{
	EAITokenSlot Slot = EAITokenSlot::Melee;
	int32 Amount = 0;
};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UAITargetComponent : public UActorComponent, public ITickDebugger
{
	GENERATED_BODY()

public:
	UAITargetComponent();

	virtual void BeginPlay() override;

	void TickDebug_Implementation( float DeltaTime, FString& OutDebugText ) override;

	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens", meta = ( ReturnDisplayName = "bSuccess" ) )
	bool ReserveTokens( UAIAttackerComponent* Reserver, EAITokenSlot Slot, int32 Tokens );
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens", meta = ( ReturnDisplayName = "bSuccess" ) )
	bool FreeTokens( UAIAttackerComponent* Reserver, int32 Tokens = 0 );
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	void ClearTokens();
	UFUNCTION( BlueprintCallable, Category = "AITarget|Tokens" )
	void SetTokenCooldown( EAITokenSlot Slot, float Seconds );

	UFUNCTION( BlueprintPure, Category = "AITarget|Tokens" )
	int32 GetReservedTokens( UAIAttackerComponent* Reserver ) const;
	UFUNCTION( BlueprintPure, Category = "AITarget|Tokens" )
	int32 GetRemainingTokens( EAITokenSlot Slot ) const;
	UFUNCTION( BlueprintPure, Category = "AITarget|Tokens" )
	float GetTokenCooldown( EAITokenSlot Slot ) const;

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
	UFUNCTION( BlueprintPure, Category = "AITarget|Attackers" )
	void GetAttackers( TArray<UAIAttackerComponent*>& Attackers ) const;

	UFUNCTION( BlueprintCallable, Category = "AITarget" )
	void FreeReservations( UAIAttackerComponent* Reserver );

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAttackersUpdate, UAITargetComponent*, AITargetComponent );
	UPROPERTY( BlueprintAssignable, Category = "AITarget" )
	FOnAttackersUpdate OnAttackersUpdate {};

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AITarget" )
	TArray<FAITargetGroupSettings> GroupsSettings {};

	UPROPERTY( EditAnywhere, Category = "AITarget", meta = ( ArraySizeEnum = "EAITokenSlot" ) )
	int32 MaxTokens[static_cast<int>( EAITokenSlot::MAX )];

private:
	float EndTokenCooldownTime[static_cast<int>( EAITokenSlot::MAX )];

	TMap<UAIAttackerComponent*, FAIReservedToken> ReservedTokens {};
	TMap<UAIAttackerComponent*, int32> ReservedGroupPlaces {};
	TSet<UAIAttackerComponent*> DeclaredAttackers {};
};
