/*
 * Implemented by Corentin Paya
 */

#include "AssetCreatorLibrary.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

#if WITH_EDITOR
#include <FileHelpers.h>
#endif

#define LOCTEXT_NAMESPACE "FEasySequencerModule"

UActorComponent* UAssetCreatorLibrary::AddComponent(
	TSubclassOf<class UActorComponent> ComponentClass,
	AActor* Actor,
	USceneComponent* ParentComponent,
	FName Name
)
{
	if ( !Actor ) return nullptr;

#if WITH_EDITOR
	Actor->RerunConstructionScripts();
#endif

	const UClass* NewComponentClass = ComponentClass.Get();
	if ( !NewComponentClass ) return nullptr;

	UActorComponent* Result = NewObject<UActorComponent>( Actor, NewComponentClass, Name );

	if ( USceneComponent* AsSceneComponent = Cast<USceneComponent>( Result ) )
	{
		AsSceneComponent->AttachToComponent(
			ParentComponent ? ParentComponent : Actor->GetRootComponent(),
			FAttachmentTransformRules::KeepRelativeTransform
		);
		AsSceneComponent->SetComponentToWorld( FTransform {} );
	}

	Actor->AddInstanceComponent( Result );
	Result->OnComponentCreated();
	Result->RegisterComponent();

	UpdateActor( Actor );
	return Result;
}

void UAssetCreatorLibrary::RemoveComponent( UActorComponent* ActorComponent )
{
	if ( !ActorComponent ) return;

	AActor* Actor = ActorComponent->GetOwner();
	Actor->RemoveInstanceComponent( ActorComponent );
	ActorComponent->DestroyComponent();

	UpdateActor( Actor );
}

void UAssetCreatorLibrary::ShowFormattedDialog( const FString& InFileName )
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

void UAssetCreatorLibrary::UpdateActor( AActor* Actor )
{
#if WITH_EDITOR
	Actor->RerunConstructionScripts();
	FEditorFileUtils::SaveCurrentLevel();
#endif

	SavePackage( Actor );
}

void UAssetCreatorLibrary::SavePackage( UObject* Object )
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
