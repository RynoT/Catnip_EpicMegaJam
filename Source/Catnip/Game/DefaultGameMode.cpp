// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"

#include "Level/Ring.h"
#include "Engine/World.h"
#include "Level/RingHandler.h"
#include "Player/CatCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/SpringArmComponent.h"

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

	this->MovementSpeed = 1600.0f;
	this->CurrentDistance = -5250.0f;
	this->InterpCameraSpeed = 5.0f;
	this->InterpCharacterSpeed = 10.0f;

	this->PlayerOffsetCache = FVector::ZeroVector;

	Super::PrimaryActorTick.bCanEverTick = true;
}

void ADefaultGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> TempArray;

	// Destroy all exisiting rings.
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ARing::StaticClass(), TempArray);
	for (int32 i = TempArray.Num() - 1; i >= 0; --i)
	{
		if (TempArray[i] != nullptr)
		{
			TempArray[i]->Destroy();
		}
	}
	TempArray.Empty();

	// Set the ring handler. It should already be placed in-world.
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ARingHandler::StaticClass(), TempArray);
	if (ensure(TempArray.Num() == 1))
	{
		this->RingHandler = Cast<ARingHandler>(TempArray[0]);
		this->RingHandler->UpdateRings();
	}
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
	} 
	else
#endif
	{
		LocationUpdate = this->RingHandler->GetLocationAtDistance(this->CurrentDistance);
	}

	ACatCharacter *Character = Cast<ACatCharacter>(Controller->GetPawn());
	if (Character != nullptr)
	{
		FRotator CharacterRotation = Character->GetActorRotation();
		FRotator RotationUpdate = this->RingHandler->GetRotationAtDistance(this->CurrentDistance);

		float RadiusShrink = Character->GetSimpleCollisionRadius();

		// Restrict PlayerOffset.
		FVector &PlayerOffset = Character->GetPlayerOffsetRef();
		{
			PlayerOffset = this->RingHandler->RestrictPosition(LocationUpdate + RotationUpdate.RotateVector(PlayerOffset), RadiusShrink);
			PlayerOffset = RotationUpdate.UnrotateVector(PlayerOffset - LocationUpdate);
		}

		// Interpolate PlayerOffset.
		{
			if (this->PlayerOffsetCache.IsNearlyZero())
			{
				this->PlayerOffsetCache = PlayerOffset;
			}
			this->PlayerOffsetCache = FMath::VInterpTo(this->PlayerOffsetCache, PlayerOffset, DeltaTime, this->InterpCharacterSpeed);
		}

		// Update character location and rotation.
		Character->SetActorLocationAndRotation(LocationUpdate + RotationUpdate.RotateVector(this->PlayerOffsetCache), RotationUpdate);


		//FVector NextLocation = LocationUpdate + RotationUpdate.RotateVector(PlayerOffset);
		//NextLocation = this->RingHandler->RestrictPosition(NextLocation);

		//Character->SetActorLocationAndRotation(NextLocation, RotationUpdate);

		//PlayerOffset = PlayerOffset.GetSafeNormal() * (NextLocation - LocationUpdate).Size();

		//// Update camera location and rotation.
		//UCameraComponent *Camera = Character->GetCamera();
		//FVector CameraLocationUpdate = Character->GetActorLocation() + RotationUpdate.RotateVector(Character->GetCameraOffset());

		//FVector TargetCameraLocation = LocationUpdate + RotationUpdate.RotateVector(Character->GetCameraOffset());
		//FVector CurrentCameraLocation = this->CameraOffset;
		//FVector NextCameraLocation = FMath::VInterpTo(CurrentCameraLocation, TargetCameraLocation, DeltaTime, this->InterpCameraSpeed);
		//Camera->SetWorldLocationAndRotation(TargetCameraLocation, RotationUpdate);
		//this->CameraOffset = NextCameraLocation;

		//// Update character location and rotation.
		//float &Distance = Character->GetDistanceRef();
		//FVector &Direction = Character->GetDirectionRef();

		//constexpr float RingPadding = 150.0f;
		//this->RingHandler
		//Distance = FMath::Min(Distance, this->RingHandler->GetRingRadius());// -RingPadding);

		//FVector TargetOffsetLocation = Direction * Distance;
		//this->PlayerOffset = FMath::VInterpTo(this->PlayerOffset, TargetOffsetLocation, DeltaTime, this->InterpCharacterSpeed);
		////this->PlayerOffset = FMath::VInterpConstantTo(this->PlayerOffset, TargetOffsetLocation, DeltaTime, this->InterpCharacterSpeed);
		////this->PlayerOffset = TargetOffsetLocation;

		//LocationUpdate += Character->GetActorRotation().RotateVector(this->PlayerOffset);
		//Character->SetActorLocationAndRotation(LocationUpdate, RotationUpdate);
	}
	this->RingHandler->UpdatePawnLocation(LocationUpdate);
}
