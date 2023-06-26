// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Learn01/Public/QuickAssetAction.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeQuickAssetAction() {}
// Cross Module References
	BLUTILITY_API UClass* Z_Construct_UClass_UAssetActionUtility();
	LEARN01_API UClass* Z_Construct_UClass_UQuickAssetAction();
	LEARN01_API UClass* Z_Construct_UClass_UQuickAssetAction_NoRegister();
	UPackage* Z_Construct_UPackage__Script_Learn01();
// End Cross Module References
	DEFINE_FUNCTION(UQuickAssetAction::execRemoveUnusedAssets)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->RemoveUnusedAssets();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UQuickAssetAction::execAddPrefix)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->AddPrefix();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(UQuickAssetAction::execDuplicateAssets)
	{
		P_GET_PROPERTY(FIntProperty,Z_Param_NumOfDuplicates);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->DuplicateAssets(Z_Param_NumOfDuplicates);
		P_NATIVE_END;
	}
	void UQuickAssetAction::StaticRegisterNativesUQuickAssetAction()
	{
		UClass* Class = UQuickAssetAction::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "AddPrefix", &UQuickAssetAction::execAddPrefix },
			{ "DuplicateAssets", &UQuickAssetAction::execDuplicateAssets },
			{ "RemoveUnusedAssets", &UQuickAssetAction::execRemoveUnusedAssets },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_UQuickAssetAction_AddPrefix_Statics
	{
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UQuickAssetAction_AddPrefix_Statics::Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "ModuleRelativePath", "Public/QuickAssetAction.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UQuickAssetAction_AddPrefix_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UQuickAssetAction, nullptr, "AddPrefix", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UQuickAssetAction_AddPrefix_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UQuickAssetAction_AddPrefix_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UQuickAssetAction_AddPrefix()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UQuickAssetAction_AddPrefix_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics
	{
		struct QuickAssetAction_eventDuplicateAssets_Parms
		{
			int32 NumOfDuplicates;
		};
		static const UECodeGen_Private::FIntPropertyParams NewProp_NumOfDuplicates;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::NewProp_NumOfDuplicates = { "NumOfDuplicates", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, nullptr, nullptr, STRUCT_OFFSET(QuickAssetAction_eventDuplicateAssets_Parms, NumOfDuplicates), METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::NewProp_NumOfDuplicates,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "ModuleRelativePath", "Public/QuickAssetAction.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UQuickAssetAction, nullptr, "DuplicateAssets", nullptr, nullptr, sizeof(Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::QuickAssetAction_eventDuplicateAssets_Parms), Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_UQuickAssetAction_RemoveUnusedAssets_Statics
	{
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_UQuickAssetAction_RemoveUnusedAssets_Statics::Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "ModuleRelativePath", "Public/QuickAssetAction.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UQuickAssetAction_RemoveUnusedAssets_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UQuickAssetAction, nullptr, "RemoveUnusedAssets", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_UQuickAssetAction_RemoveUnusedAssets_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_UQuickAssetAction_RemoveUnusedAssets_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_UQuickAssetAction_RemoveUnusedAssets()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UQuickAssetAction_RemoveUnusedAssets_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UQuickAssetAction);
	UClass* Z_Construct_UClass_UQuickAssetAction_NoRegister()
	{
		return UQuickAssetAction::StaticClass();
	}
	struct Z_Construct_UClass_UQuickAssetAction_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UQuickAssetAction_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAssetActionUtility,
		(UObject* (*)())Z_Construct_UPackage__Script_Learn01,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_UQuickAssetAction_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_UQuickAssetAction_AddPrefix, "AddPrefix" }, // 1473929956
		{ &Z_Construct_UFunction_UQuickAssetAction_DuplicateAssets, "DuplicateAssets" }, // 1009490601
		{ &Z_Construct_UFunction_UQuickAssetAction_RemoveUnusedAssets, "RemoveUnusedAssets" }, // 1467918666
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UQuickAssetAction_Statics::Class_MetaDataParams[] = {
		{ "Comment", "/**\n * \n */" },
		{ "HideCategories", "Object" },
		{ "IncludePath", "QuickAssetAction.h" },
		{ "ModuleRelativePath", "Public/QuickAssetAction.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UQuickAssetAction_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UQuickAssetAction>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UQuickAssetAction_Statics::ClassParams = {
		&UQuickAssetAction::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		0,
		0,
		0x001000A0u,
		METADATA_PARAMS(Z_Construct_UClass_UQuickAssetAction_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UQuickAssetAction_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UQuickAssetAction()
	{
		if (!Z_Registration_Info_UClass_UQuickAssetAction.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UQuickAssetAction.OuterSingleton, Z_Construct_UClass_UQuickAssetAction_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UQuickAssetAction.OuterSingleton;
	}
	template<> LEARN01_API UClass* StaticClass<UQuickAssetAction>()
	{
		return UQuickAssetAction::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UQuickAssetAction);
	UQuickAssetAction::~UQuickAssetAction() {}
	struct Z_CompiledInDeferFile_FID_EnhancedEditor_Plugins_Learn01_Source_Learn01_Public_QuickAssetAction_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_EnhancedEditor_Plugins_Learn01_Source_Learn01_Public_QuickAssetAction_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_UQuickAssetAction, UQuickAssetAction::StaticClass, TEXT("UQuickAssetAction"), &Z_Registration_Info_UClass_UQuickAssetAction, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UQuickAssetAction), 960916854U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_EnhancedEditor_Plugins_Learn01_Source_Learn01_Public_QuickAssetAction_h_988523884(TEXT("/Script/Learn01"),
		Z_CompiledInDeferFile_FID_EnhancedEditor_Plugins_Learn01_Source_Learn01_Public_QuickAssetAction_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_EnhancedEditor_Plugins_Learn01_Source_Learn01_Public_QuickAssetAction_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
