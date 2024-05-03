#include "MXHUtilityLibrary.h"

//  TODO: Move these CVars definitions inside a FunctionLibrary dedicated for CVars?

static TAutoConsoleVariable<bool> CVarAIDebug(
    TEXT( "MXH.AI.Debug" ),
    false,
    TEXT( "Set debug mode for AI.\n" ),
    ECVF_Default 
);

bool UMXHUtilityLibrary::IsCVarAIDebugEnabled()
{
    return CVarAIDebug.GetValueOnAnyThread();
}
