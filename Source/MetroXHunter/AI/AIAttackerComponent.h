/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIAttackerComponent.generated.h"

class UAITargetComponent;

/*
 *
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UAIAttackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIAttackerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason );

	UFUNCTION( BlueprintCallable, Category = "AIAttacker" )
	void SetCurrentTarget( UAITargetComponent* Target );
	UFUNCTION( BlueprintCallable, Category = "AIAttacker" )
	void FreeReservations();

	UFUNCTION( BlueprintCallable, Category = "AIAttacker" )
	void SetGroupPlace( int32 NewGroupIndex );
	UFUNCTION( BlueprintPure, Category = "AIAttacker" )
	int32 GetGroupPlace() const;

	/*
	 * Returns the reserver actor that should be used for AITargetComponent.
	 * By default, it returns the component's owner.
	 */
	UFUNCTION( BlueprintCallable, Category = "AIAttacker" )
	AActor* GetReserver() const;
	UFUNCTION( BlueprintCallable, Category = "AIAttacker" )
	UAITargetComponent* GetCurrentTarget() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGroupPlaceChanged, int32, LastGroupIndex, int32, NewGroupIndex );
	UPROPERTY( BlueprintAssignable, Category = "AIAttacker" )
	FOnGroupPlaceChanged OnGroupPlaceChanged;

private:
	UAITargetComponent* CurrentTarget = nullptr;
	int32 GroupIndex = -1;
};
