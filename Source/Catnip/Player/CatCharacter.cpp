// Fill out your copyright notice in the Description page of Project Settings.


#include "CatCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACatCharacter::ACatCharacter()
{
	// Set size for collision capsule
	Super::GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Set our turn rates for input.
	this->BaseTurnRate = 45.0f;
	this->BaseLookUpRate = 45.0f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	Super::bUseControllerRotationPitch = false;
	Super::bUseControllerRotationYaw = false;
	Super::bUseControllerRotationRoll = false;

	// Configure character movement.
	Super::GetCharacterMovement()->bOrientRotationToMovement = true;
	Super::GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Super::GetCharacterMovement()->JumpZVelocity = 600.f;
	Super::GetCharacterMovement()->AirControl = 0.2f;

	// Create a Camera camera.
	this->Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CatCamera"));
	this->Camera->SetupAttachment(Super::RootComponent);
	this->Camera->bUsePawnControlRotation = false;

	// Disable physics.
	Super::GetCapsuleComponent()->SetSimulatePhysics(false);
}

void ACatCharacter::SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent)
{
	check(PlayerInputComponent);

	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//PlayerInputComponent->BindAxis("MoveForward", this, &ACatCharacter::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &ACatCharacter::MoveRight);

	//PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	//PlayerInputComponent->BindAxis("TurnRate", this, &ACatCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &ACatCharacter::LookUpAtRate);
}

void ACatCharacter::TurnAtRate(float Rate)
{
	Super::AddControllerYawInput(Rate * this->BaseTurnRate * Super::GetWorld()->GetDeltaSeconds());
}

void ACatCharacter::LookUpAtRate(float Rate)
{
	Super::AddControllerPitchInput(Rate * this->BaseLookUpRate * Super::GetWorld()->GetDeltaSeconds());
}

void ACatCharacter::MoveForward(float Value)
{
	if (Super::Controller != nullptr && !FMath::IsNearlyZero(Value))
	{
		// Find out which way is forward.
		const FRotator Rotation = Super::Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		// Get forward vector.
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		Super::AddMovementInput(Direction, Value);
	}
}

void ACatCharacter::MoveRight(float Value)
{
	if (Super::Controller != nullptr && !FMath::IsNearlyZero(Value))
	{
		// Find out which way is right.
		const FRotator Rotation = Super::Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		// Get right vector.
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		Super::AddMovementInput(Direction, Value);
	}
}

