// Copyright Epic Games, Inc. All Rights Reserved.

#include "DMT_AI_V2.h"

#define LOCTEXT_NAMESPACE "FDMT_AI_V2Module"

void FDMT_AI_V2Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FDMT_AI_V2Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDMT_AI_V2Module, DMT_AI_V2)