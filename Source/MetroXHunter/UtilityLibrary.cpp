#include "UtilityLibrary.h"

//  TODO: Move these CVars definitions inside a FunctionLibrary dedicated for CVars?

static TAutoConsoleVariable<bool> CVarAIDebug(
    TEXT( "MXH.AI.Debug" ),
    false,
    TEXT( "Set debug mode for AI.\n" ),
    ECVF_Default
);

bool UUtilityLibrary::SaveFileAsString( const FString& Data, const FString& Path )
{
    return FFileHelper::SaveStringToFile( Data, *Path );
}

bool UUtilityLibrary::LoadFileAsString( const FString& Path, FString& Data )
{
    return FFileHelper::LoadFileToString( Data, *Path );
}

bool UUtilityLibrary::IsWithinEditor()
{
#if WITH_EDITOR
    return true;
#else
    return false;
#endif
}

bool UUtilityLibrary::IsCVarAIDebugEnabled()
{
    return CVarAIDebug.GetValueOnAnyThread();
}

FString UUtilityLibrary::GetProjectVersion()
{
    FString Value = "";
    GConfig->GetString(
        TEXT( "/Script/EngineSettings.GeneralProjectSettings" ),
        TEXT( "ProjectVersion" ),
        Value,
        GGameIni
    );

    return Value;
}
