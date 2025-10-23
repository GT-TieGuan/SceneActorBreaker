// Copyright Epic Games, Inc. All Rights Reserved.

#include "SceneActorBreakerStyle.h"
#include "SceneActorBreaker.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FSceneActorBreakerStyle::StyleInstance = nullptr;

void FSceneActorBreakerStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSceneActorBreakerStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FSceneActorBreakerStyle::GetStyleSetName()
{
	static FName StyleIdentifier(TEXT("SceneActorBreakerStyle"));
	return StyleIdentifier;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef<FSlateStyleSet> FSceneActorBreakerStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("SceneActorBreakerStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SceneActorBreaker")->GetBaseDir() / TEXT("Resources"));

	Style->Set("SceneActorBreaker.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FSceneActorBreakerStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FSceneActorBreakerStyle::Get()
{
	return *StyleInstance;
}