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
	        "GameplayTasks"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");

        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");	

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
