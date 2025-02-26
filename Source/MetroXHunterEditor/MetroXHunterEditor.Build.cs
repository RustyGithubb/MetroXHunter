using UnrealBuildTool;

public class MetroXHunterEditor : ModuleRules
{
    public MetroXHunterEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // Removed: Type = ModuleType.Editor;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "InputCore",
            "MetroXHunter"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",
            "PropertyEditor",
            "EditorStyle"
        });
    }
}
