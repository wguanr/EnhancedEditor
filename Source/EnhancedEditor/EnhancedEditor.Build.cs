// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class EnhancedEditor : ModuleRules
{
	public EnhancedEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new[]
			{
				// ... add other private include paths required here ...
				Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Editor/Blutility/Private"
			}
		);


		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core", "Blutility", "EditorScriptingUtilities", "UMG", "Niagara", "UnrealEd", "AssetTools",
				"ContentBrowser", "InputCore"
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore", "EditorStyle"

				// ... add private dependencies that you statically link with here ...	
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}