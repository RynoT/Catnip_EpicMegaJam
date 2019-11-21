// Fill out your copyright notice in the Description page of Project Settings.


#include "CatCharacter.h"

#include "Game/DefaultGameMode.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACatCharacter::ACatCharacter()
{
	this->TiltSpeed = 8.0f;
	this->TiltVerticalValue = 18.0f;
	this->TiltHorizontalValue = 18.0f;

	this->PlayerOffset = FVector::ZeroVector;
	this->CameraOffset = FVector::ZeroVector; // Set in BeginPlay.

	// Create spring arm.
	this->SpringArm = UObject::CreateDefaultSubobject<USpringArmComponent>(TEXT("CatSpringArm"));
	this->SpringArm->TargetArmLength = 600.0f;
	this->SpringArm->SetupAttachment(Super::RootComponent, USpringArmComponent::SocketName);

	// Create camera.
	this->Camera = UObject::CreateDefaultSubobject<UCameraComponent>(TEXT("CatCamera"));
	this->Camera->bUsePawnControlRotation = false;
	this->Camera->SetupAttachment(this->SpringArm);

	// Disable use controllor rotation.
	Super::bUseControllerRotationYaw = false;
	Super::bUseControllerRotationRoll = false;
	Super::bUseControllerRotationPitch = false;

	// Disable physics.
	Super::GetCapsuleComponent()->SetSimulatePhysics(false);

	// Set size for collision capsule.
	Super::GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	Super::PrimaryActorTick.bCanEverTick = true;
}

void ACatCharacter::BeginPlay()
{
	Super::BeginPlay();

	check(this->Camera != nullptr);
	this->CameraOffset = this->Camera->GetRelativeTransform().GetLocation();

	check(Super::GetMesh() != nullptr);
	this->InitialRotation = Super::GetMesh()->GetRelativeTransform().GetRotation().Rotator();
}

void ACatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	USkeletalMeshComponent *SkeletalMesh = Super::GetMesh();
	SkeletalMesh->SetRelativeRotation(this->InitialRotation);
	SkeletalMesh->AddLocalRotation(this->Tilt);
}

void ACatCharacter::Action()
{
	ADefaultGameMode *GameMode = Super::GetWorld()->GetAuthGameMode<ADefaultGameMode>();
	check(GameMode != nullptr);
	GameMode->RegisterAction();
}

void ACatCharacter::MoveUp(float Value)
{
	float DeltaTime = Super::GetWorld()->GetDeltaSeconds();
	if (FMath::IsNearlyZero(Value))
	{
		this->Tilt.Roll = FMath::FInterpTo(this->Tilt.Roll, 0.0f , DeltaTime, this->TiltSpeed);
		return;
	}
	this->Tilt.Roll = FMath::FInterpTo(this->Tilt.Roll, this->TiltVerticalValue * FMath::Clamp(Value, -1.0f, 1.0f), DeltaTime, this->TiltSpeed);
	this->PlayerOffset += FVector::UpVector * Value;
}

void ACatCharacter::MoveRight(float Value)
{
	float DeltaTime = Super::GetWorld()->GetDeltaSeconds();
	if (FMath::IsNearlyZero(Value))
	{
		this->Tilt.Yaw = FMath::FInterpTo(this->Tilt.Yaw, 0.0f, DeltaTime, this->TiltSpeed);
		return;
	}
	this->Tilt.Yaw = FMath::FInterpTo(this->Tilt.Yaw, this->TiltHorizontalValue * FMath::Clamp(Value, -1.0f, 1.0f), DeltaTime, this->TiltSpeed);
	this->PlayerOffset += FVector::RightVector * Value;
}

void ACatCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &ACatCharacter::MoveUp);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACatCharacter::MoveRight);

	PlayerInputComponent->BindAction(TEXT("Action"), EInputEvent::IE_Pressed, this, &ACatCharacter::Action);
}
