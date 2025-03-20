using UnrealBuildTool;

public class GoapShooter : ModuleRules
{
    public GoapShooter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "HeadMountedDisplay",
            "EnhancedInput",
            "AIModule",
            "NavigationSystem",
            "GameplayTasks",
            "GameplayTags",
            "GameplayAbilities",
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
            "AIModule"
        });

        // Add all the necessary include paths based on our project structure
        PublicIncludePaths.AddRange(new string[] {
            // Core game systems
            "GoapShooter/Public/Core",
            // Character system
            "GoapShooter/Public/Characters",
            // Controller system
            "GoapShooter/Public/Controllers",
            // AI and GOAP system
            "GoapShooter/Public/AI",
            "GoapShooter/Public/AI/GOAP",
            "GoapShooter/Public/AI/GOAP/Actions",
            "GoapShooter/Public/AI/GOAP/Goals",
            "GoapShooter/Public/AI/GOAP/WorldStates"
        });

        PrivateIncludePaths.AddRange(new string[] {
            // Core game systems
            "GoapShooter/Private/Core",
            // Character system
            "GoapShooter/Private/Characters",
            // Controller system
            "GoapShooter/Private/Controllers",
            // AI and GOAP system
            "GoapShooter/Private/AI",
            "GoapShooter/Private/AI/GOAP",
            "GoapShooter/Private/AI/GOAP/Actions",
            "GoapShooter/Private/AI/GOAP/Goals"
        });
    }
}
