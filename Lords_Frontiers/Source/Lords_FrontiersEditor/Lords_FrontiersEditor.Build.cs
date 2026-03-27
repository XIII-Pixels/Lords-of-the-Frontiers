using UnrealBuildTool;

public class Lords_FrontiersEditor : ModuleRules
{
    public Lords_FrontiersEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "Slate",
            "SlateCore",
            "Blutility",
            "UMGEditor",
            "UnrealEd",
            "Lords_Frontiers"
        });
    }
}
