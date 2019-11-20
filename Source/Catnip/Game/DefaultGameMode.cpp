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

	this->LifeCount = 9;

	this->MovementSpeed = 1600.0f;
	//this->CurrentDistance = -5250.0f;
	this->InterpCameraSpeed = 5.0f;
	this->InterpCharacterSpeed = 10.0f;

	this->PlayerOffsetCache = FVector::ZeroVector;

	Super::bStartPlayersAsSpectators = true;
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
}

void ADefaultGameMode::FindRingHandler()
{
	TArray<AActor*> TempArray;

	// Set the ring handler. It should already be placed in-world.
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ARingHandler::StaticClass(), TempArray);
	if (ensure(TempArray.Num() == 1))
	{
		this->RingHandler = Cast<ARingHandler>(TempArray[0]);

		this->RingHandler->OnBeatRingFail.AddDynamic(this, &ADefaultGameMode::OnBeatRingFail);
		this->RingHandler->OnBeatRingSuccess.AddDynamic(this, &ADefaultGameMode::OnBeatRingSuccess);

		this->CurrentDistance = -this->RingHandler->GetFadeDistance();
	}
}

void ADefaultGameMode::OnBeatRingFail(int32 RingIndex)
{
	//UE_LOG(LogTemp, Log, TEXT("FAIL %d"), RingIndex);
	--this->LifeCount;
}

void ADefaultGameMode::OnBeatRingSuccess(int32 RingIndex)
{
	//UE_LOG(LogTemp, Log, TEXT("SUCCESS %d"), RingIndex);
}

void ADefaultGameMode::RegisterAction()
{
	if (!ensure(this->RingHandler != nullptr))
	{
		return;
	}
	this->RingHandler->RegisterAction();

	//APlayerController *Controller = Super::GetWorld()->GetFirstPlayerController();
	//check(controller != nullptr);
	//APawn *Pawn = Controller->GetPawn();
	//if (Pawn == nullptr)
	//{
	//	return;
	//}
	//float PawnDistance = this->RingHandler->GetCurrentPawnDistance();
	//const TArray<int32> &BeatRings = this->RingHandler->GetBeatRings();

	//// Find nearest beat ring.
	//int32 NearestBeatRing;
	//float DistanceToNearest;
	//for (int32 i = 0; i < BeatRings.Num(); ++i)
	//{
	//	if (BeatRings[i] < this->LastCompletedBeat)
	//	{
	//		continue;
	//	}
	//	float Distance = FMath::Abs(PawnDistance - (BeatRings[i] * this->RingHandler->GetRingDistance()));
	//	if (i == 0 || Distance < DistanceToNearest)
	//	{
	//		NearestBeatRing = BeatRings[i];
	//		DistanceToNearest = Distance;
	//	}
	//}

	//// Check if the nearest ring is the one just completed, or if we are not within range. Fail if so.
	//if (NearestBeatRing == this->LastCompletedBeat || DistanceToNearest > this->ActionDistanceAllowance)
	//{
	//	this->OnBeatFail(NearestBeatRing);
	//	return;
	//}

	//// If here, then we were successful.
	//this->OnBeatSuccess(NearestBeatRing);
	//this->LastCompletedBeat = NearestBeatRing;
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

		float RadiusShrink = Character->GetSimpleCollisionRadius() * 2.0f;

		// Restrict PlayerOffset.
		FVector &PlayerOffset = Character->GetPlayerOffsetRef();
		{
			PlayerOffset = this->RingHandler->RestrictPositionOffset(LocationUpdate, PlayerOffset, RadiusShrink);
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
	}
	//this->RingHandler->UpdatePawnLocation(LocationUpdate);

	this->RingHandler->UpdateHandler(LocationUpdate);
}
