// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UDWTN : ModuleRules
{
	public UDWTN(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "Sockets", "Networking", "OnlineSubsystem", "UMG", "HTTP", "Json", "JsonUtilities" });
        //소켓통신과 네트워크에 필요한 모듈 "Sockets", "Networking"
        //Json 파싱을 위한 모듈 "Json", "JsonUtilities"
        //웹서버의 정보를 받아와야하기 때문에 필요한 모듈 "HTTP"
        //위젯을 사용하기 위한 모듈 "UMG"

        //PrivateDependencyModuleNames.AddRange(new string[] {  });

        // Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
        //위젯을 사용하기 위해서는 추가로 PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" }); 부분이 필요함
    }
}
