// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CatCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class CATNIP_API ACatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACatCharacter();

protected:
	virtual void BeginPlay() override;

public:
	void MoveUp(float Value);
	void MoveRight(float Value);

	FORCEINLINE FVector &GetPlayerOffsetRef()
	{
		return this->PlayerOffset;
	}

	FORCEINLINE UCameraComponent *GetCamera()
	{
		return this->Camera;
	}

	FORCEINLINE const FVector &GetCameraOffset() const
	{
		return this->CameraOffset;
	}

protected:
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent *Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent *SpringArm;

private:
	FVector CameraOffset;
	FVector PlayerOffset;
};
