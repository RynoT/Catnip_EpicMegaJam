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

	void MoveUp(float Value);
	void MoveRight(float Value);

	FORCEINLINE float &GetDistanceRef()
	{
		return this->Distance;
	}

	FORCEINLINE FVector &GetDirectionRef()
	{
		return this->Direction;
	}

	FORCEINLINE UCameraComponent *GetCamera()
	{
		return this->Camera;
	}

protected:
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent *Camera;

private:
	float Distance;
	FVector Direction;
};
