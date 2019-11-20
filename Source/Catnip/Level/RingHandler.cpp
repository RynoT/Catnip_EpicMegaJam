// Fill out your copyright notice in the Description page of Project Settings.


#include "RingHandler.h"

#include "Ring.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "ConstructorHelpers.h"
#include "Components/SplineComponent.h"

#define CONSTRUCTOR_RING_CLASS TEXT("/Game/Blueprints/Level/BP_Ring")

ARingHandler::ARingHandler()
{
	static ConstructorHelpers::FClassFinder<ARing> ConstructorRingClass = ConstructorHelpers::FClassFinder<ARing>(CONSTRUCTOR_RING_CLASS);
	this->RingClass = ensure(ConstructorRingClass.Succeeded()) ? ConstructorRingClass.Class : ARing::StaticClass();

	this->CurrentPawnDistance = 0.0f;
	this->NextBeatRingIndex = -1;
	this->bNextBeatRingCompleted = false;
	this->BeatActionDistanceAllowance = 650.0f;

	this->RingDistance = 500.0f;
	this->RingFadeDistance = 8000.0f;
	this->bDebugUpdateRings = false;
	this->bDebugDeleteRings = false;

	this->RingSpawnRadius = 500.0f;
	this->RingSpawnResolution = 12;
	this->RingSpawnRotationOffset = PI * 2.0f;
	this->RingSpawnRotateSpeedMin = -25.0f;
	this->RingSpawnRotateSpeedMax = 25.0f;

	this->SceneComponent = UObject::CreateDefaultSubobject<USceneComponent>(TEXT("HandlerSceneComponent"));
	Super::RootComponent = this->SceneComponent;

	this->SplineComponent = UObject::CreateDefaultSubobject<USplineComponent>(TEXT("HandlerSplineComponent"));
	this->SplineComponent->SetMobility(EComponentMobility::Movable);
	this->SplineComponent->SetClosedLoop(false, false);
	this->SplineComponent->SetupAttachment(Super::RootComponent);

	Super::PrimaryActorTick.bCanEverTick = false;
}

FVector ARingHandler::GetLocationAtDistance(float Distance) const
{
	check(this->SplineComponent != nullptr);
	if (Distance < 0.0f)
	{
		FVector Location = this->SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		FVector Direction = this->SplineComponent->GetDirectionAtSplinePoint(0, ESplineCoordinateSpace::World);
		return Location + Direction * Distance;
	}
	return this->SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FRotator ARingHandler::GetRotationAtDistance(float Distance) const
{
	check(this->SplineComponent != nullptr);
	if (Distance < 0.0f)
	{
		return this->SplineComponent->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);
	}
	return this->SplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FVector ARingHandler::FindLocationClosestTo(FVector Location) const
{
	check(this->SplineComponent != nullptr);
	return this->SplineComponent->FindLocationClosestToWorldLocation(Location, ESplineCoordinateSpace::World);
}

void ARingHandler::BeginPlay()
{
	Super::BeginPlay();

	for (int32 i = 0; i < this->Rings.Num(); ++i)
	{
		if (this->Rings[i] != nullptr)
		{
			this->Rings[i]->Destroy();
		}
	}
	this->Rings.Empty();
	this->ActiveRules.Empty();

	this->SpawnState.Color = FColor(45, 195, 220);
	this->SpawnState.Mesh = this->RingMeshDefault;
	this->SpawnState.MeshType = ERingMeshType::MultipleMesh;
	this->SpawnState.Radius = this->RingSpawnRadius;
	this->SpawnState.Resolution = this->RingSpawnResolution;
	this->SpawnState.RotationOffset = this->RingSpawnRotationOffset;
	this->SpawnState.OffsetType = ERingOffsetType::Random;
	this->SpawnState.RotationSpeedMin = this->RingSpawnRotateSpeedMin;
	this->SpawnState.RotationSpeedMax = this->RingSpawnRotateSpeedMax;
	this->SpawnState.RotationForceRerollMin = -1.0f;
	this->SpawnState.MaterialInterface = this->RingMaterialInterface;
}

void ARingHandler::RegisterAction()
{
	if (this->NextBeatRingIndex == -1 || this->bNextBeatRingCompleted)
	{
		if (this->CurrentPawnDistance > 0.0f)
		{
			this->OnBeatRingFail.Broadcast(-1);
		}
	}
	else
	{
		this->bNextBeatRingCompleted = true;
		this->OnBeatRingSuccess.Broadcast(this->NextBeatRingIndex);
	}
}

FVector ARingHandler::RestrictPositionOffset(const FVector &SplinePosition, const FVector &PositionOffset, float RadiusShrink) const
{
	if (PositionOffset.SizeSquared() <= FMath::Square(this->RingSpawnRadius - RadiusShrink))
	{
		return PositionOffset;
	}
	return PositionOffset.GetSafeNormal() * (this->RingSpawnRadius - RadiusShrink);
}

#if 0
void ARingHandler::UpdateRings()
{
	this->DeleteRings();

	if (true)
	{
		return;
	}
	if (!ensure(this->RingDistance > 0.0f))
	{
		return;
	}
	if (!ensure(this->RingStaticMeshes.Num() > 0))
	{
		return;
	}

	float Length = this->SplineComponent->GetSplineLength();
	for (float i = 0.0f; i < Length; i += this->RingDistance)
	{
		FVector Location = this->SplineComponent->GetLocationAtDistanceAlongSpline(i, ESplineCoordinateSpace::World);
		FRotator Rotation = this->SplineComponent->GetRotationAtDistanceAlongSpline(i, ESplineCoordinateSpace::World);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ARing *Ring = Super::GetWorld()->SpawnActor<ARing>(this->RingClass, Location, Rotation, Params);
		ensure(Ring != nullptr);
		Ring->UpdatePoints(this->RingStaticMeshes[FMath::RandRange(0, this->RingStaticMeshes.Num() - 1)], this->RingRadius);

		this->Rings.Add(Ring);
	}
}

void ARingHandler::DeleteRings()
{
	if (this->Rings.Num() == 0)
	{
		return;
	}
	for (int32 i = this->Rings.Num() - 1; i >= 0; --i)
	{
		if (this->Rings[i] == nullptr)
		{
			continue;
		}
		this->Rings[i]->Destroy();
	}
	this->Rings.Empty();
}
#endif

ARingHandler* ARingHandler::SpawnRule_SetRadius(int32 OnRing, float NewRadius, int32 TransitionRings)
{
	this->AddSpawnRule(OnRing, FRingSpawnRule::CreateLambda([=](FRingSpawnState &SpawnState, FActiveRingSpawnRule &SpawnRule)
		{
			float &Memory = *SpawnRule.GetCache<float>();
			if (SpawnRule.RingCounter <= 1)
			{
				Memory = SpawnState.Radius;
			}
			float Percentage = TransitionRings <= 0 ? 1.0f : FMath::Clamp(SpawnRule.RingCounter / float(TransitionRings + 1), 0.0f, 1.0f);
			float NextRadius = Memory + (NewRadius - Memory) * FMath::Sin(Percentage * PI / 2.0f);
			SpawnState.Radius = NextRadius;
			return SpawnRule.RingCounter > TransitionRings;
		}));
	return this;
}

ARingHandler* ARingHandler::SpawnRule_SetMesh(int32 OnRing, UStaticMesh *NewMesh, UMaterialInterface *NewMaterial, ERingMeshType Type, bool bSingleRing)
{
	this->AddSpawnRule(OnRing, FRingSpawnRule::CreateLambda([=](FRingSpawnState &SpawnState, FActiveRingSpawnRule &SpawnRule)
		{
			UStaticMesh *&MeshMemory = *SpawnRule.GetCache<UStaticMesh*>();
			ERingMeshType &MeshTypeMemory = *SpawnRule.GetCache<ERingMeshType, sizeof(UStaticMesh*)>();
			UMaterialInterface *&MeshMaterialMemory = *SpawnRule.GetCache<UMaterialInterface*, sizeof(UStaticMesh*) + sizeof(ERingMeshType)>();
			if (SpawnRule.RingCounter <= 1)
			{
				MeshMemory = SpawnState.Mesh;
				MeshTypeMemory = SpawnState.MeshType;
				MeshMaterialMemory = SpawnState.MaterialInterface;
			}

			SpawnState.Mesh = NewMesh;
			SpawnState.MeshType = Type;
			SpawnState.MaterialInterface = NewMaterial;

			if (!bSingleRing)
			{
				return true;
			}
			if (SpawnRule.RingCounter >= 2)
			{
				SpawnState.Mesh = MeshMemory;
				SpawnState.MeshType = MeshTypeMemory;
				SpawnState.MaterialInterface = MeshMaterialMemory;
				return true;
			}
			return false;
		}));
	return this;
}

ARingHandler* ARingHandler::SpawnRule_SetOffset(int32 OnRing, float Value, ERingOffsetType Type)
{
	this->AddSpawnRule(OnRing, FRingSpawnRule::CreateLambda([=](FRingSpawnState &SpawnState, FActiveRingSpawnRule &SpawnRule)
		{
			SpawnState.RotationOffset = Value;
			SpawnState.OffsetType = Type;
			if (Type == ERingOffsetType::Incremental)
			{
				SpawnState.OffsetCounter = 0.0f;
			}
			return true;
		}));
	return this;
}

ARingHandler* ARingHandler::SpawnRule_SetRotation(int32 OnRing, float MinSpeed, float MaxSpeed, float ForceRerollMin)
{
	this->AddSpawnRule(OnRing, FRingSpawnRule::CreateLambda([=](FRingSpawnState &SpawnState, FActiveRingSpawnRule &SpawnRule)
		{
			SpawnState.RotationSpeedMin = MinSpeed;
			SpawnState.RotationSpeedMax = MaxSpeed;
			SpawnState.RotationForceRerollMin = ForceRerollMin;
			return true;
		}));
	return this;
}

ARingHandler* ARingHandler::SpawnRule_SetColor(int32 OnRing, FColor Color, bool bSingleRing)
{
	this->AddSpawnRule(OnRing, FRingSpawnRule::CreateLambda([=](FRingSpawnState &SpawnState, FActiveRingSpawnRule &SpawnRule)
		{
			FColor &ColorMemory = *SpawnRule.GetCache<FColor>();
			if (SpawnRule.RingCounter <= 1)
			{
				ColorMemory = SpawnState.Color;
			}
			SpawnState.Color = Color;

			if (!bSingleRing)
			{
				return true;
			}
			if (SpawnRule.RingCounter >= 2)
			{
				SpawnState.Color = ColorMemory;
				return true;
			}
			return false;
		}));
	return this;
}

ARingHandler* ARingHandler::SpawnRule_SetResolution(int32 OnRing, int32 Resolution)
{
	this->AddSpawnRule(OnRing, FRingSpawnRule::CreateLambda([=](FRingSpawnState &SpawnState, FActiveRingSpawnRule &SpawnRule)
		{
			SpawnState.Resolution = Resolution;
			return true;
		}));
	return this;
}

ARingHandler* ARingHandler::SpawnRule_SetBeatRings(FString Input, TArray<UStaticMesh*> Meshes, UMaterialInterface *MeshMaterial, FColor Color)
{
	TArray<FString> StrArray;
	Input.ParseIntoArray(StrArray, TEXT(","), true);

	TArray<int32> NumArray;
	NumArray.Reserve(StrArray.Num());
	
	for (int32 i = 0; i < StrArray.Num(); ++i)
	{
		int32 Num = FCString::Atoi(*StrArray[i].TrimStartAndEnd());
		if (NumArray.Contains(Num))
		{
			continue;
		}
		NumArray.Add(Num);
	}
	NumArray.Sort();

	this->BeatSpawnState.Rings = NumArray;
	this->BeatSpawnState.Meshes = Meshes;
	this->BeatSpawnState.MaterialInterface = MeshMaterial;
	this->BeatSpawnState.Color = Color;
	return this;
}

ARing* ARingHandler::SpawnRing(int32 Index)
{
	// Increment rule ring counter.
	for (FActiveRingSpawnRule &Next : this->ActiveRules)
	{
		if (Next.RingIndex <= Index)
		{
			++Next.RingCounter;
		}
	}

	// Execute all rules.
	for (int32 i = 0; i < this->ActiveRules.Num(); ++i)
	{
		FActiveRingSpawnRule &ActiveRule = this->ActiveRules[i];
		if (ActiveRule.RingIndex > Index)
		{
			continue;
		}
		if (ActiveRule.Rule.Execute(this->GetSpawnState(), ActiveRule))
		{
			this->ActiveRules.RemoveAtSwap(i--);
		}
	}

	// Spawn the ring. The above rules will have set the conditions for us.
	float Distance = this->RingDistance * Index;

	FVector Location = this->SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	FRotator Rotation = this->SplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARing *Ring = Super::GetWorld()->SpawnActor<ARing>(this->RingClass, Location, Rotation, Params);
	ensure(Ring != nullptr);
	//UE_LOG(LogTemp, Log, TEXT("%d, %f"), Index, this->SpawnRadius);
	//Ring->UpdatePoints(this->SpawnMesh, this->SpawnRingType == ERingType::SingleMesh, this->SpawnRadius);
	Ring->InitRing(&this->GetSpawnState());
	return Ring;
}

void ARingHandler::UpdateHandler(FVector PawnLocation)
{
	float InputKey = this->SplineComponent->FindInputKeyClosestToWorldLocation(PawnLocation);
	float DistanceAtLocation = this->GetDistanceAtInputKey(InputKey);
	if (FMath::IsNearlyZero(DistanceAtLocation))
	{
		DistanceAtLocation = -(this->SplineComponent->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World) - PawnLocation).Size();
	}
	float SplineLength = this->SplineComponent->GetSplineLength();
	check(SplineLength > 0);
	float CurrentPercentage = DistanceAtLocation / SplineLength;

	const float FadeTolerance = this->RingDistance * 2.0f;
	const float AppearTolerance = this->RingDistance * 2.0f;
	float MinDistance = DistanceAtLocation - AppearTolerance;
	float MaxDistance = DistanceAtLocation + this->RingFadeDistance;

	const int32 MaxRings = FMath::CeilToInt(SplineLength / this->RingDistance);
	int32 MinRing = FMath::Clamp(int32(MinDistance / this->RingDistance), 0, MaxRings);
	int32 MaxRing = FMath::Clamp(int32(MaxDistance / this->RingDistance) + 1, 0, MaxRings);

	this->CurrentPawnDistance = DistanceAtLocation;

	// Find next beat ring.
	int32 NextBeatRing = -1;
	for (int32 i = 0; i < this->BeatSpawnState.Rings.Num(); ++i)
	{
		if (this->NextBeatRingIndex != -1 && this->BeatSpawnState.Rings[i] < this->NextBeatRingIndex)
		{
			continue;
		}
		if (FMath::Abs(this->BeatSpawnState.Rings[i] * this->RingDistance
			- DistanceAtLocation) <= this->BeatActionDistanceAllowance)
		{
			NextBeatRing = this->BeatSpawnState.Rings[i];
			break;
		}
	}
	if (NextBeatRing == -1 && this->NextBeatRingIndex != -1)
	{
		if (!this->bNextBeatRingCompleted)
		{
			this->OnBeatRingFail.Broadcast(this->NextBeatRingIndex);
		}
		else
		{
			this->bNextBeatRingCompleted = false;
		}
		this->NextBeatRingIndex = -1;
	}
	if (NextBeatRing != -1)
	{
		this->NextBeatRingIndex = NextBeatRing;
	}

	// Remove unneeded rings. Update transparency of needed ones.
	for (int32 i = 0; i < this->Rings.Num(); ++i)
	{
		ARing *Ring = this->Rings[i];
		if (Ring != nullptr)
		{
			int32 Index = Ring->GetRingIndex();
			if (Index >= MinRing && Index <= MaxRing)
			{
				// This ring is needed. Update its transparency.
				float Percentage = (Index * this->RingDistance - FadeTolerance) / SplineLength;
				float Opacity =  FMath::Clamp((Percentage - CurrentPercentage) 
					/ (this->RingFadeDistance / SplineLength), 0.0f, 1.0f);
				this->Rings[i]->UpdateRingOpacity(1.0f - Opacity);
				//UE_LOG(LogTemp, Log, TEXT("%f"), Opacity);
				//this->Rings[i]->UpdateRingOpacity(FMath::Sin((1.0f - Opacity) * PI * 0.5f));

				continue;
			}
			Ring->Destroy();
		}
		this->Rings.RemoveAtSwap(i--);
	}
	//UE_LOG(LogTemp, Log, TEXT("----"));

	// Spawn any required new rings.
	for (int32 i = MinRing; i <= MaxRing; ++i)
	{
		// Don't spawn if ring exists. Only spawn if previous ring exists (spawn linearly).
		bool bFound = false, bCanSpawn = i == 0;
		for (ARing *Next : this->Rings)
		{
			if (Next == nullptr)
			{
				continue;
			}
			if (Next->GetRingIndex() == i - 1)
			{
				bCanSpawn = true;
			}
			if (Next->GetRingIndex() == i)
			{
				bFound = true;
				break;
			}
		}
		if (bFound || !bCanSpawn)
		{
			continue;
		}

		// Check if ring is a beat ring.
		if (this->BeatSpawnState.Rings.Contains(i + 1))
		{
			this->SpawnRule_SetMesh(i + 1, this->BeatSpawnState.Meshes[FMath::RandRange(0, this->BeatSpawnState
				.Meshes.Num() - 1)], this->BeatSpawnState.MaterialInterface, ERingMeshType::SingleMesh, true);
			this->SpawnRule_SetColor(i + 1, this->BeatSpawnState.Color, true);
		}

		// If we get here it means we have to spawn the ring. First ensure any rules are set as active.
		if (this->SpawnRuleMap.Contains(i))
		{
			for (FRingSpawnRule &Rule : this->SpawnRuleMap[i])
			{
				this->ActiveRules.Add(FActiveRingSpawnRule{ i, 0, Rule });
			}
		}

		ARing *Ring = this->SpawnRing(i);
		if (ensure(Ring != nullptr))
		{
			Ring->SetRingIndex(i);
			this->Rings.Add(Ring);
		}
	}
}

#if 0
void ARingHandler::UpdatePawnLocation(FVector Location)
{
	float InputKey = this->SplineComponent->FindInputKeyClosestToWorldLocation(Location);
	float DistanceAtLocation = this->GetDistanceAtInputKey(InputKey);
	if (FMath::IsNearlyZero(DistanceAtLocation))
	{
		DistanceAtLocation = -(this->SplineComponent->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World) - Location).Size();
	}
	float SplineLength = this->SplineComponent->GetSplineLength();
	check(SplineLength > 0);
	float CurrentPercentage = DistanceAtLocation / SplineLength;

	const float FadePercentage = this->RingFadeDistance / SplineLength;
	const float AppearPercentageOffset = 250.0f / SplineLength;
	const float DisappearPercentageOffset = 1000.0f / SplineLength;

	for (int32 i = 0; i < this->Rings.Num(); ++i)
	{
		if (this->Rings[i] == nullptr)
		{
			continue;
		}
		float Distance = this->RingDistance * i;
		float Percentage = Distance / this->SplineComponent->GetSplineLength() - AppearPercentageOffset;

		float Opacity;
		if (Percentage < CurrentPercentage - DisappearPercentageOffset)
		{
			Opacity = 0.0f;
		}
		else
		{
			Opacity = 1.0f - FMath::Clamp((Percentage - CurrentPercentage) / FadePercentage, 0.0f, 1.0f);
		}
		this->Rings[i]->UpdateRingOpacity(Opacity);
	}
}
#endif

float ARingHandler::GetDistanceAtInputKey(float InputKey) const
{
	const TArray<FInterpCurvePointFloat> &Points = this->SplineComponent->SplineCurves.ReparamTable.Points;

	int32 UpperBound = 0;
	int32 Count = Points.Num();
	while (Count > 0)
	{
		int32 Step = Count / 2;
		int32 Itr = UpperBound + Step;
		if (InputKey >= Points[Itr].OutVal)
		{
			UpperBound = ++Itr;
			Count -= Step + 1;
			continue;
		}
		Count = Step;
	}

	if (UpperBound == 0)
	{
		return Points[0].InVal;
	}
	if (UpperBound == Points.Num())
	{
		return Points.Last().InVal;
	}

	const FInterpCurvePointFloat &P0 = Points[UpperBound - 1], &P1 = Points[UpperBound];
	return FMath::Lerp(P0.InVal, P1.InVal, (InputKey - P0.OutVal) / (P1.OutVal - P0.OutVal));
}

#if WITH_EDITOR
void ARingHandler::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName Name = Event.MemberProperty != nullptr ? Event.MemberProperty->GetFName() : NAME_None;
	//if (Name == GET_MEMBER_NAME_CHECKED(ARingHandler, bDebugUpdateRings))
	//{
	//	this->UpdateRings();
	//	this->bDebugUpdateRings = false;
	//}
	//if (Name == GET_MEMBER_NAME_CHECKED(ARingHandler, bDebugDeleteRings))
	//{
	//	this->DeleteRings();
	//	this->bDebugDeleteRings = false;
	//}
}
#endif