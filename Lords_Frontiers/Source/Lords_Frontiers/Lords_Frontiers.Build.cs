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

        // Localization translations are read at runtime from the loose CSVs in Content/Localization
        // (GameLocalization.cpp -> FFileHelper::LoadFileToString). Raw .csv files are NOT cooked, so
        // without this they never reach a packaged build: FFileHelper fails, no translations get
        // registered, and language switching silently does nothing in a build even though it works in
        // the editor. Stage them explicitly (UFS / pak) so ProjectContentDir()/Localization/*.csv
        // resolves at runtime in the build.
        RuntimeDependencies.Add("$(ProjectDir)/Content/Localization/*.csv", StagedFileType.UFS);

        OptimizeCode = CodeOptimization.InShippingBuildsOnly;

        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");	

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
