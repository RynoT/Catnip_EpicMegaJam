// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ring.generated.h"

struct FRingSpawnState;
class USplineComponent;

UCLASS()
class CATNIP_API ARing : public AActor
{
	GENERATED_BODY()
	
public:	
	ARing();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	virtual void Tick(float DeltaTime) override;

	void InitRing(FRingSpawnState *State);

	void InitObstacle(FRingSpawnState *State);

	//void UpdateColor(FLinearColor Color);

	//void UpdatePoints(UStaticMesh *Mesh, bool bSingleMesh, float Radius);

	void UpdateRingOpacity(float RingOpacity);

	UFUNCTION()
	void OnObstacleOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, 
		UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	FORCEINLINE int32 GetRingIndex() const
	{
		return this->RingIndex;
	}

	FORCEINLINE void SetRingIndex(int32 Index)
	{
		this->RingIndex = Index;
	}

	FORCEINLINE float GetRingRadius() const
	{
		return this->RingRadius;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeedMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeedMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeedRerollZone;

	UPROPERTY(VisibleAnywhere)
	USceneComponent *SceneComponent;

	UPROPERTY(VisibleAnywhere)
	USplineComponent *SplineComponent;

	UPROPERTY()
	UMaterialInstanceDynamic *MaterialInstanceDynamic;

	UPROPERTY()
	TArray<UStaticMeshComponent*> StaticMeshComponents;

	UPROPERTY()
	UStaticMeshComponent *ObstacleMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	bool bDebugDisableRotation;

private:
	int32 RingIndex;
	float RingRadius;

	float LastOpacity;
	float RequiredOpacity;

	//bool bVisible;
	bool bObstacleHit;
	float RotateSpeed;
};

/// INLINE ///

FORCEINLINE void ARing::UpdateRingOpacity(float RingOpacity)
{
	this->RequiredOpacity = RingOpacity;
}