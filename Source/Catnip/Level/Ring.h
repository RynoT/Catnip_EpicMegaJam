// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ring.generated.h"

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

	void UpdateColor(FLinearColor Color);

	void UpdatePoints(UStaticMesh *Mesh, float Radius);

	void UpdateRingOpacity(float RingOpacity);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RingResolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeedMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeedMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeedRerollZone;

	UPROPERTY()
	UStaticMeshComponent *StaticMeshComponent;

	UPROPERTY(VisibleAnywhere)
	USceneComponent *SceneComponent;

	UPROPERTY(VisibleAnywhere)
	USplineComponent *SplineComponent;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface *MaterialInterface;

	UPROPERTY()
	UMaterialInstanceDynamic *MaterialInstanceDynamic;

	UPROPERTY(EditDefaultsOnly)
	bool bDebugDisableRotation;

private:
	float LastOpacity;
	float RequiredOpacity;

	bool bVisible;
	float RotateSpeed;
};

/// INLINE ///

FORCEINLINE void ARing::UpdateRingOpacity(float RingOpacity)
{
	this->RequiredOpacity = RingOpacity;
}