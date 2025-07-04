/*
 * Implemented by BARRAU Benoit
 */

#pragma once

/**
* A macro to enable script execution guard
* to execute blueprint events if running in editor
*/
#if WITH_EDITOR
#define EXECUTE_GUARDED(code) if(!GAllowActorScriptExecutionInEditor) { FEditorScriptExecutionGuard ScriptGuard; code } else { code }
#else
#define EXECUTE_GUARDED(code) code
#endif // WITH_EDITOR
