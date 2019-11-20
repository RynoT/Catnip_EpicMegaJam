// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RingHandler.generated.h"

class ARing;
class UStaticMesh;
class USplineComponent;
struct FRingSpawnState;
struct FActiveRingSpawnRule;

DECLARE_DELEGATE_RetVal_TwoParams(bool, FRingSpawnRule, FRingSpawnState &, FActiveRingSpawnRule &);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeatRingFail, int32, RingIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeatRingSuccess, int32, RingIndex);

UENUM(BlueprintType)
enum class ERingMeshType : uint8
{
	SingleMesh, MultipleMesh
};

UENUM(BlueprintType)
enum class ERingOffsetType : uint8
{
	Random, Fixed, Incremental
};

USTRUCT()
struct FActiveRingSpawnRule
{
	GENERATED_BODY()

public:
	int32 RingIndex = -1;
	int32 RingCounter = 0;

	FRingSpawnRule Rule;

	uint8 CacheMemory[18];

	template<typename T, uint64 Offset = 0>
	FORCEINLINE T* GetCache()
	{
		return reinterpret_cast<T*>(&this->CacheMemory[Offset]);
	}
};

USTRUCT()
struct FRingSpawnState
{
	GENERATED_BODY()

public:
	float Radius;
	int32 Resolution;
	FColor Color;

	float RotationSpeedMin;
	float RotationSpeedMax;
	float RotationForceRerollMin;

	float RotationOffset;
	ERingOffsetType OffsetType;
	float OffsetCounter;

	UPROPERTY()
	UStaticMesh *Mesh;
	ERingMeshType MeshType;

	UPROPERTY()
	UMaterialInterface *MaterialInterface;

	bool bSpawnObstacle;

	UPROPERTY()
	UStaticMesh *ObstacleMesh;

	UPROPERTY()
	UMaterialInterface *ObstacleMaterialInterface;
};

USTRUCT()
struct FRingBeatSpawnState
{
	GENERATED_BODY()

public:
	FColor Color;
	TArray<int32> Rings;

	UPROPERTY()
	TArray<UStaticMesh*> Meshes;

	UPROPERTY()
	UMaterialInterface *MaterialInterface;

	UPROPERTY()
	TArray<UStaticMesh*> ObstacleMeshes;

	UPROPERTY()
	UMaterialInterface *ObstacleMaterialInterface;
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
	void FailRing(int32 Ring);

	void RegisterAction();

	void UpdateHandler(FVector PawnLocation);

	float GetDistanceAtInputKey(float InputKey) const;

	FVector GetLocationAtDistance(float Distance) const;

	FRotator GetRotationAtDistance(float Distance) const;

	FVector FindLocationClosestTo(FVector Location) const;

	FVector RestrictPositionOffset(const FVector &SplinePosition, const FVector &PositionOffset, float RadiusShrink = 0.0f) const;

	void AddSpawnRule(int32 OnRing, FRingSpawnRule Rule);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetBeatRings(FString Input, TArray<UStaticMesh*> Meshes, UMaterialInterface *MeshMaterial,
		FColor Color, TArray<UStaticMesh*> ObstacleMeshes, UMaterialInterface *ObstacleMaterialInterface);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetRadius(int32 OnRing, float NewRadius, int32 TransitionRings = 0);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetMesh(int32 OnRing, UStaticMesh *NewMesh, UMaterialInterface *NewMaterial, ERingMeshType Type, bool bSingleRing = true);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetOffset(int32 OnRing, float Value, ERingOffsetType Type);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetRotation(int32 OnRing, float MinSpeed, float MaxSpeed, float ForceRerollMin = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetColor(int32 OnRing, FColor Color, bool bSingleRing = false);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler* SpawnRule_SetResolution(int32 OnRing, int32 Resolution = 12);

	UFUNCTION(BlueprintCallable, Category = "Spawn Rules")
	ARingHandler *SpawnRule_SetObstacle(int32 OnRing, UStaticMesh *ObstacleMesh, UMaterialInterface *ObstacleMaterial);

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

	/// ///

	ARing *SpawnRing(int32 Index);

	UFUNCTION(BlueprintPure, Category = "RingHandler")
	FORCEINLINE float GetFadeDistance() const
	{
		return RingFadeDistance;
	}

	UFUNCTION(BlueprintPure, Category = "RingHandler")
	FORCEINLINE float GetRingDistance() const
	{
		return this->RingDistance;
	}

	FORCEINLINE float GetCurrentPawnDistance() const
	{
		return this->CurrentPawnDistance;
	}

	FORCEINLINE FRingSpawnState& GetSpawnState()
	{
		return this->SpawnState;
	}

	FORCEINLINE const TArray<int32> &GetBeatRings() const
	{
		return this->BeatSpawnState.Rings;
	}

protected:
	UPROPERTY(EditDefaultsOnly)
	float BeatActionDistanceAllowance;

	UPROPERTY(EditDefaultsOnly)
	float ObstacleSpawnChancePercentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingFadeDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingSpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RingSpawnResolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingSpawnRotateSpeedMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingSpawnRotateSpeedMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RingSpawnRotationOffset;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ARing> RingClass;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh *RingMeshDefault;	
	
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface *RingMaterialInterface;

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

public:
	/// EVENTS ///

	UPROPERTY(BlueprintAssignable, Category = "Rings")
	FOnBeatRingFail OnBeatRingFail;

	UPROPERTY(BlueprintAssignable, Category = "Rings")
	FOnBeatRingSuccess OnBeatRingSuccess;

private:
	int32 NextBeatRingIndex;
	int32 LastFailRing;
	//bool bNextBeatRingCompleted;

	float CurrentPawnDistance;

	UPROPERTY()
	FRingSpawnState SpawnState;

	UPROPERTY()
	FRingBeatSpawnState BeatSpawnState;

	UPROPERTY()
	TArray<FActiveRingSpawnRule> ActiveRules;
	TMap<int32, TArray<FRingSpawnRule>> SpawnRuleMap;
};

/// INLINE ///

FORCEINLINE void ARingHandler::AddSpawnRule(int32 OnRing, FRingSpawnRule Rule)
{
	--OnRing;

	TArray<FRingSpawnRule> *Array;
	if (this->SpawnRuleMap.Contains(OnRing))
	{
		Array = &this->SpawnRuleMap[OnRing];
	}
	else
	{
		Array = &this->SpawnRuleMap.Add(OnRing, TArray<FRingSpawnRule>());
	}
	check(Array != nullptr);
	Array->Add(Rule);
}