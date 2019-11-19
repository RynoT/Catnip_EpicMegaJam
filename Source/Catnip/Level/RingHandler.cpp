// Fill out your copyright notice in the Description page of Project Settings.


#include "RingHandler.h"

#include "Ring.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ConstructorHelpers.h"
#include "Components/SplineComponent.h"

#define CONSTRUCTOR_RING_CLASS TEXT("/Game/Blueprints/Level/BP_Ring")

ARingHandler::ARingHandler()
{
	static ConstructorHelpers::FClassFinder<ARing> ConstructorRingClass = ConstructorHelpers::FClassFinder<ARing>(CONSTRUCTOR_RING_CLASS);
	this->RingClass = ensure(ConstructorRingClass.Succeeded()) ? ConstructorRingClass.Class : ARing::StaticClass();

	this->RingRadius = 500.0f;
	this->RingDistance = 500.0f;
	this->RingFadeDistance = 8000.0f;
	this->bDebugUpdateRings = false;
	this->bDebugDeleteRings = false;

	this->SceneComponent = UObject::CreateDefaultSubobject<USceneComponent>(TEXT("HandlerSceneComponent"));
	Super::RootComponent = this->SceneComponent;

	this->SplineComponent = UObject::CreateDefaultSubobject<USplineComponent>(TEXT("HandlerSplineComponent"));
	this->SplineComponent->SetMobility(EComponentMobility::Movable);
	this->SplineComponent->SetClosedLoop(false, false);
	this->SplineComponent->SetupAttachment(Super::RootComponent);

	Super::PrimaryActorTick.bCanEverTick = true;
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
}

void ARingHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector ARingHandler::RestrictPositionOffset(const FVector &SplinePosition, const FVector &PositionOffset, float RadiusShrink) const
{
	if (PositionOffset.SizeSquared() <= FMath::Square(this->RingRadius - RadiusShrink))
	{
		return PositionOffset;
	}
	return PositionOffset.GetSafeNormal() * (this->RingRadius - RadiusShrink);
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

ARing* ARingHandler::SpawnRing(int32 Index)
{
	float Distance = this->RingDistance * Index;

	FVector Location = this->SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	FRotator Rotation = this->SplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARing *Ring = Super::GetWorld()->SpawnActor<ARing>(this->RingClass, Location, Rotation, Params);
	ensure(Ring != nullptr);
	Ring->UpdatePoints(this->RingStaticMeshes[FMath::RandRange(0, this->RingStaticMeshes.Num() - 1)], this->RingRadius);
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
		// Check if ring currently exists.
		bool bFound = false;
		for (ARing *Next : this->Rings)
		{
			if (Next != nullptr && Next->GetRingIndex() == i)
			{
				bFound = true;
				break;
			}
		}
		if (bFound)
		{
			continue;
		}

		// If we get here it means we have to spawn the ring.
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