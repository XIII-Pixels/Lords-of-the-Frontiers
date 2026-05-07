// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Lords_Frontiers : ModuleRules
{
    public Lords_Frontiers(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
	        "Core",
	        "CoreUObject",
	        "Engine",
	        "InputCore",
	        "UMG",
	        "NavigationSystem",
	        "Slate",
	        "SlateCore",
	        "EnhancedInput",
	        "Json",
	        "JsonUtilities",
	        "Niagara",
	        "AIModule",
	        "GameplayTasks",
	        "GeometryCache",
	        "GameplayTags"
        });

		PrivateDependencyModuleNames.AddRange( new string[] { "ProceduralMeshComponent" } );

        AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");

        OptimizeCode = CodeOptimization.InShippingBuildsOnly;

        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");	

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
