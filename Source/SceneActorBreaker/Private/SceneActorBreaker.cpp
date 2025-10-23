// Copyright GT-TieGuan. All Rights Reserved.
#include "SceneActorBreaker.h"
#include "SceneActorBreakerStyle.h"
#include "SceneActorBreakerCommands.h"
#include "AssetSelection.h"
#include "Selection.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"

static const FName ActorDecomposerTabName("SceneActorBreaker");

#define LOCTEXT_NAMESPACE "FSceneActorBreakerModule"

void FSceneActorBreakerModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("Initializing Scene Actor Breaker Module..."));

	FSceneActorBreakerStyle::Initialize();
	FSceneActorBreakerStyle::ReloadTextures();

	FSceneActorBreakerCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSceneActorBreakerCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FSceneActorBreakerModule::ExecuteDecompositionButtonClicked),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSceneActorBreakerModule::RegisterMenus));

	UE_LOG(LogTemp, Log, TEXT("Scene Actor Breaker Module initialized successfully"));
}

void FSceneActorBreakerModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FSceneActorBreakerStyle::Shutdown();
	FSceneActorBreakerCommands::Unregister();
}

void FSceneActorBreakerModule::ExecuteDecompositionButtonClicked()
{
	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);

	if (SelectedActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No actors selected for decomposition"));
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSelectionWarning", "Please select one or more actors to break."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Starting decomposition of %d selected actors"), SelectedActors.Num());

	GEditor->BeginTransaction(LOCTEXT("ActorDecompositionTransaction", "Decompose Actors into StaticMeshComponents"));

	int32 TotalNewActors = 0;
	for (AActor* Actor : SelectedActors)
	{
		TotalNewActors += ProcessActorDecomposition(Actor) ? 1 : 0;
	}

	GEditor->EndTransaction();

	GEditor->RedrawLevelEditingViewports();

	UE_LOG(LogTemp, Log, TEXT("Decomposition completed. Created %d new actors"), TotalNewActors);

}

bool FSceneActorBreakerModule::ProcessActorDecomposition(AActor* TargetActor)
{
	if (!TargetActor || !TargetActor->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid actor provided for decomposition"));
		return false;
	}

	UE_LOG(LogTemp, Verbose, TEXT("Processing actor decomposition: %s"), *TargetActor->GetName());

	TSet<UActorComponent*> ActorComponents = TargetActor->GetComponents();
	TArray<AActor*> GeneratedActors;
	int32 ComponentCounter = 0;

	for (UActorComponent* Component : ActorComponents)
	{
		UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Component);

		if (MeshComponent && MeshComponent != TargetActor->GetRootComponent() && MeshComponent->GetStaticMesh())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.bNoFail = true;
			SpawnParams.ObjectFlags = RF_Transactional;

			AStaticMeshActor* NewMeshActor = TargetActor->GetWorld()->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(),
				MeshComponent->GetComponentTransform(),
				SpawnParams
			);

			if (NewMeshActor)
			{
				FString NewActorName = FString::Printf(TEXT("%s_Part_%d"), *TargetActor->GetActorLabel(), ++ComponentCounter);
				NewMeshActor->SetActorLabel(NewActorName);

				UStaticMeshComponent* NewMeshComp = NewMeshActor->GetStaticMeshComponent();
				if (NewMeshComp)
				{
					NewMeshComp->SetStaticMesh(MeshComponent->GetStaticMesh());

					for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); MaterialIndex++)
					{
						NewMeshComp->SetMaterial(MaterialIndex, MeshComponent->GetMaterial(MaterialIndex));
					}

					NewMeshComp->SetWorldTransform(MeshComponent->GetComponentTransform());
					NewMeshComp->SetCollisionEnabled(MeshComponent->GetCollisionEnabled());
					NewMeshComp->SetCollisionProfileName(MeshComponent->GetCollisionProfileName());
					NewMeshComp->SetMobility(MeshComponent->Mobility);
					NewMeshComp->RegisterComponent();

					GeneratedActors.Add(NewMeshActor);

					UE_LOG(LogTemp, Verbose, TEXT("Created decomposed actor: %s"), *NewActorName);
				}
			}
		}
	}

	if (GeneratedActors.Num() > 0)
	{
		GEditor->SelectNone(false, true);
		for (AActor* NewActor : GeneratedActors)
		{
			GEditor->SelectActor(NewActor, true, false);
		}
		UE_LOG(LogTemp, Log, TEXT("Generated %d new actors from decomposition"), GeneratedActors.Num());
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("No valid mesh components found for decomposition in actor: %s"), *TargetActor->GetName());
	return false;
}

void FSceneActorBreakerModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& MenuSection = WindowMenu->FindOrAddSection("WindowLayout");
			MenuSection.AddMenuEntryWithCommandList(FSceneActorBreakerCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& ToolbarSection = Toolbar->FindOrAddSection("PluginTools");
			FToolMenuEntry& ToolbarEntry = ToolbarSection.AddEntry(
				FToolMenuEntry::InitToolBarButton(FSceneActorBreakerCommands::Get().PluginAction)
			);
			ToolbarEntry.SetCommandList(PluginCommands);
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSceneActorBreakerModule, SceneActorBreaker)
