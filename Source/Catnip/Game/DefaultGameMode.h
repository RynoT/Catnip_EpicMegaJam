// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DefaultGameMode.generated.h"

class ARingHandler;

/**
 * 
 */
UCLASS()
class CATNIP_API ADefaultGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ADefaultGameMode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void RegisterAction();

	UFUNCTION()
	void OnBeatRingFail(int32 RingIndex);

	UFUNCTION()
	void OnBeatRingSuccess(int32 RingIndex);

	FORCEINLINE ARingHandler* GetRingHandler()
	{
		return this->RingHandler;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpCameraSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpCharacterSpeed;

	UPROPERTY()
	ARingHandler *RingHandler;

private:
	float CurrentDistance;

	FVector PlayerOffsetCache;
};
