// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RingHandler.generated.h"

class ARing;
class UStaticMesh;
class USplineComponent;

DECLARE_DELEGATE_RetVal_ThreeParams(bool, FRingSpawnRule, ARingHandler*, int32, float&);

UENUM(BlueprintType)
enum class ERingType : uint8
{
	MultipleMesh, SingleMesh
};

USTRUCT()
struct FActiveRingSpawnRule
{
	GENERATED_BODY()

public:
	int32 RingIndex = -1;
	int32 RingCounter = 0;

	FRingSpawnRule Rule;

	float CacheMemory;
};

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

	void UpdateHandler(FVector PawnLocation);

	float GetDistanceAtInputKey(float InputKey) const;

	FVector GetLocationAtDistance(float Distance) const;

	FRotator GetRotationAtDistance(float Distance) const;

	FVector FindLocationClosestTo(FVector Location) const;

	FVector RestrictPositionOffset(const FVector &SplinePosition, const FVector &PositionOffset, float RadiusShrink = 0.0f) const;

	void AddSpawnRule(int32 OnRing, FRingSpawnRule Rule);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetRadius(int32 OnRing, float NewRadius, int32 TransitionRings = 0);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetMesh(int32 OnRing, UStaticMesh *NewMesh, ERingType Type, bool bSingleRing = true);

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

	/// ///

	ARing *SpawnRing(int32 Index);

	float GetRingRadius() const;
	void SetRingRadius(float Radius);

	UStaticMesh *GetDefaultRingMesh() const;
	void SetRingMesh(UStaticMesh *Mesh, ERingType Type);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingFadeDistance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ARing> RingClass;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh *RingMeshDefault;

	//UPROPERTY(EditDefaultsOnly)
	//TArray<UStaticMesh*> RingStaticMeshes;

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

private:
	float SpawnRadius;
	ERingType SpawnRingType;
	UStaticMesh *SpawnMesh;

	UPROPERTY()
	TArray<FActiveRingSpawnRule> ActiveRules;

//	UPROPERTY()
	TMap<int32, TArray<FRingSpawnRule>> SpawnRuleMap;
};

/// INLINE ///

FORCEINLINE float ARingHandler::GetRingRadius() const
{
	return this->SpawnRadius;
}

FORCEINLINE void ARingHandler::SetRingRadius(float Radius)
{
	this->SpawnRadius = Radius;
}

FORCEINLINE UStaticMesh *ARingHandler::GetDefaultRingMesh() const
{
	return this->RingMeshDefault;
}

FORCEINLINE void ARingHandler::SetRingMesh(UStaticMesh *Mesh, ERingType Type)
{
	this->SpawnMesh = Mesh;
	this->SpawnRingType = Type;
}