#include "MXHUtilityLibrary.h"

//  TODO: Move these CVars definitions inside a FunctionLibrary dedicated for CVars?

static TAutoConsoleVariable<bool> CVarAIDebug(
    TEXT( "MXH.AI.Debug" ),
    false,
    TEXT( "Set debug mode for AI.\n" ),
    ECVF_Default 
);

bool UMXHUtilityLibrary::SaveFileAsString( const FString& Data, const FString& Path )
{
    return FFileHelper::SaveStringToFile( Data, *Path );
}

bool UMXHUtilityLibrary::LoadFileAsString( const FString& Path, FString& Data )
{
    return FFileHelper::LoadFileToString( Data, *Path );
}

bool UMXHUtilityLibrary::IsWithinEditor()
{
#if WITH_EDITOR
    return true;
#else
    return false;
#endif
}

bool UMXHUtilityLibrary::IsCVarAIDebugEnabled()
{
    return CVarAIDebug.GetValueOnAnyThread();
}
