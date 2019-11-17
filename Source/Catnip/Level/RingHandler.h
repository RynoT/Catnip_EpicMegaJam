// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RingHandler.generated.h"

class ARing;
class UStaticMesh;
class USplineComponent;

UCLASS()
class CATNIP_API ARingHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	ARingHandler();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void UpdateRings();

	void DeleteRings();

	void UpdatePawnLocation(FVector Location);

	FVector GetLocationAtDistance(float Distance) const;

	FRotator GetRotationAtDistance(float Distance) const;

	FVector FindLocationClosestTo(FVector Location) const;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

	FORCEINLINE float GetRingRadius() const
	{
		return this->RingRadius;
	}

private:
	float GetDistanceAtInputKey(float InputKey) const;

protected:
	UPROPERTY(EditAnywhere)
	float RingRadius;

	UPROPERTY(EditDefaultsOnly)
	float RingDistance;

	UPROPERTY(EditDefaultsOnly)
	float RingFadeDistance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ARing> RingClass;

	UPROPERTY(EditDefaultsOnly)
	TArray<UStaticMesh*> RingStaticMeshes;

	UPROPERTY()
	TArray<ARing*> Rings;

	UPROPERTY(VisibleAnywhere)
	USceneComponent *SceneComponent;
	
	UPROPERTY(VisibleAnywhere)
	USplineComponent *SplineComponent;

	UPROPERTY(EditAnywhere)
	bool bDebugUpdateRings;

	UPROPERTY(EditAnywhere)
	bool bDebugDeleteRings;
};
