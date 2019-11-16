// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "Engine/World.h"
#include "Level/RingHandler.h"
#include "Player/CatCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#if WITH_EDITOR
#include "EditorLevelLibrary.h"
#include "Camera/PlayerCameraManager.h"
#endif

ADefaultGameMode::ADefaultGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PawnClass(TEXT("/Game/Blueprints/Player/BP_CatCharacter"));
	if (PawnClass.Class != nullptr)
	{
		Super::DefaultPawnClass = PawnClass.Class;
	}

	this->MovementSpeed = 1500.0f;
	this->CurrentDistance = 0.0f;

	Super::PrimaryActorTick.bCanEverTick = true;
}

void ADefaultGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ARingHandler::StaticClass(), Actors);

	if (ensure(Actors.Num() == 1))
	{
		this->RingHandler = Cast<ARingHandler>(Actors[0]);
	}

	//ensure(Actors.Num() <= 1);
	//if (Actors.Num() == 0)
	//{
	//	this->RingHandler = Super::GetWorld()->SpawnActor<ARingHandler>(ARingHandler::StaticClass());
	//	ensure(this->RingHandler != nullptr);
	//}
}

void ADefaultGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->RingHandler == nullptr)
	{
		return;
	}

	this->CurrentDistance += this->MovementSpeed * DeltaTime;

	APlayerController *Controller = Super::GetWorld()->GetFirstPlayerController();

	FVector LocationUpdate;
#if WITH_EDITOR
	APlayerCameraManager *CameraManager = Controller->PlayerCameraManager;
	if (CameraManager->GetCameraLocation().IsNearlyZero())
	{
		FVector TempLocation;
		FRotator TempRotation;
		UEditorLevelLibrary::GetLevelViewportCameraInfo(TempLocation, TempRotation);

		LocationUpdate = this->RingHandler->FindLocationClosestTo(TempLocation);
	} else
#endif
	LocationUpdate = this->RingHandler->GetLocationAtDistance(this->CurrentDistance);

	APawn *Pawn = Controller->GetPawn();
	if (Pawn != nullptr)
	{
		Pawn->SetActorLocation(LocationUpdate);
	}
	this->RingHandler->UpdatePawnLocation(LocationUpdate);
}
