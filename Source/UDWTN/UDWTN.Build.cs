// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UDWTN : ModuleRules
{
	public UDWTN(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "Sockets", "Networking", "OnlineSubsystem"});
        //소켓통신과 네트워크에 필요한 모듈 "Sockets", "Networking"
    }
}
