// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers.h"

#include "Localization/GameLocalization.h"
#include "Modules/ModuleManager.h"

class FLordsFrontiersGameModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
		LordsFrontiersLoc::Initialize();
	}

	virtual void ShutdownModule() override
	{
		LordsFrontiersLoc::Shutdown();
		FDefaultGameModuleImpl::ShutdownModule();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FLordsFrontiersGameModule, Lords_Frontiers, "Lords_Frontiers" );
