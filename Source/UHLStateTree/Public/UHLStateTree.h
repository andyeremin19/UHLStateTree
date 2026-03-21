// Pavel Penkov 2025 All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

UHLSTATETREE_API DECLARE_LOG_CATEGORY_EXTERN(LogUHLStateTree, Log, All);

class FUHLStateTreeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
