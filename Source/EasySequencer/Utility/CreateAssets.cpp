/*
 * Implemented by Corentin Paya
 */

#include "CreateAssets.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

#if WITH_EDITOR
#include <FileHelpers.h>
#endif

#define LOCTEXT_NAMESPACE "FEasySequencerModule"

UActorComponent* UCreateAssets::AddComponent( TSubclassOf<class UActorComponent> ComponentClass, AActor* Actor, USceneComponent* ParentComponent, FName Name )
{
	if ( !Actor ) return nullptr;

#if WITH_EDITOR
	Actor->RerunConstructionScripts();
#endif

	const UClass* NewComponentClass = ComponentClass.Get();
	if ( !NewComponentClass ) return nullptr;

	UActorComponent* Result = nullptr;
	Result = NewObject<UActorComponent>( (UObject*)Actor, NewComponentClass, Name );
	USceneComponent* AsSceneComponent = Cast<USceneComponent>( Result );

	if ( AsSceneComponent )
	{
		AsSceneComponent->AttachToComponent( 
			ParentComponent ? ParentComponent : Actor->GetRootComponent(),
			FAttachmentTransformRules::KeepRelativeTransform
		);
		FTransform T;
		AsSceneComponent->SetComponentToWorld( T );
	}

	Actor->AddInstanceComponent( Result );
	Result->OnComponentCreated();
	Result->RegisterComponent();

	UpdateActor( Actor );
	return Result;
}

void UCreateAssets::RemoveComponent( UActorComponent* ActorComponent, AActor* Actor )
{
	if ( !Actor || !ActorComponent ) return;

	Actor->RemoveInstanceComponent( ActorComponent );
	ActorComponent->DestroyComponent();

	UpdateActor( Actor );
}

void UCreateAssets::ShowFormattedDialog( const FString& InFileName )
{
	// Very Fancy Dialoge Prompt !
	FText DialogText = FText::Format
	(
		LOCTEXT( "PluginButtonDialogText", "{0}" ),
		FText::FromString( InFileName )
	);

#if WITH_EDITOR
	FMessageDialog::Open( EAppMsgType::Ok, DialogText );
#endif
}

void UCreateAssets::UpdateActor( AActor* Actor )
{
#if WITH_EDITOR
	Actor->RerunConstructionScripts();
	FEditorFileUtils::SaveCurrentLevel();
#endif

	SavePackage( Actor );
}

void UCreateAssets::SavePackage( UObject* Object )
{
	if ( !Object )
	{
		UE_LOG( LogTemp, Error, TEXT( "Invalid object pointer" ) );
		return;
	}

	// Get the package associated with the object
	UPackage* Package = Object->GetPackage();

	if ( !Package )
	{
		Package = Object->GetExternalPackage();
		if ( !Package ) return;
	}

	// Modify the package to mark it as dirty
	Package->Modify();
	Package->MarkPackageDirty();
}
