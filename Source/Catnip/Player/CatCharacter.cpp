// Fill out your copyright notice in the Description page of Project Settings.


#include "CatCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACatCharacter::ACatCharacter()
{
	this->Distance = 0.0f;
	this->Direction = FVector::ZeroVector;

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
}

void ACatCharacter::BeginPlay()
{
	Super::BeginPlay();

	check(this->Camera != nullptr);
	this->CameraOffset = this->Camera->GetRelativeTransform().GetLocation();
}

void ACatCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveUp", this, &ACatCharacter::MoveUp);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACatCharacter::MoveRight);
}

void ACatCharacter::MoveUp(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}
	FVector CurrentLocation = this->Direction * this->Distance;
	CurrentLocation += FVector::UpVector * Value;

	this->Direction = CurrentLocation.GetSafeNormal();
	this->Distance = CurrentLocation.Size();
}

void ACatCharacter::MoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}
	FVector CurrentLocation = this->Direction * this->Distance;
	CurrentLocation += FVector::RightVector * Value;

	this->Direction = CurrentLocation.GetSafeNormal();
	this->Distance = CurrentLocation.Size();
}

