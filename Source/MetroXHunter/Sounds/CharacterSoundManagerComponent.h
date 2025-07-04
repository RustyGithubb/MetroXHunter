/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterSoundManagerComponent.generated.h"

UENUM( BlueprintType )
enum class ECharacterOutfitSound : uint8
{
	Walk,
	Run,
	Interact,
	InteractZip,
	Misc,
	Gun,
};

UCLASS( Abstract, Blueprintable, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UCharacterSoundManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterSoundManagerComponent();

	/*
	 * Called each time a player animation notifies of a footstep event.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnFootstep( bool bIsRunning );

	/*
	 * Called when an outfit anim notify is triggered.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnOutfit( ECharacterOutfitSound OutfitType );

	/*
	 * Called when the flashlight change its state.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnFlashlightEnabled( bool bIsActivated, FVector FlashlightLocation );

	/*
	 * Called when the heal animation started.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnHealStarted();
	/*
	 * Called when the syringe from the heal animation is hitting the character's leg.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnHealImpact();

	/*
	 * Called when the feet from the stomp animation is hitting the ground.
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnStompImpact();

	/*
	 * Called when the syringe hit the ground
	 */
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent )
	void OnSyringeImpact( UStaticMeshComponent* Syringe );
};