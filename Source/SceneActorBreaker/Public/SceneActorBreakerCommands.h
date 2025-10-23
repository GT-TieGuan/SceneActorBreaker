// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SceneActorBreakerStyle.h"

class FSceneActorBreakerCommands : public TCommands<FSceneActorBreakerCommands>
{
public:
	FSceneActorBreakerCommands()
		: TCommands<FSceneActorBreakerCommands>(
			TEXT("SceneActorBreaker"),
			NSLOCTEXT("Contexts", "SceneActorBreaker", "Scene Actor Break Tool"),
			NAME_None,
			FSceneActorBreakerStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() final;

	TSharedPtr<FUICommandInfo> PluginAction;
};