// Copyright Epic Games, Inc. All Rights Reserved.

#include "SceneActorBreakerCommands.h"

#define LOCTEXT_NAMESPACE "FSceneActorBreakerModule"

void FSceneActorBreakerCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "SceneActorBreaker", "Separate actor into individual mesh components", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE