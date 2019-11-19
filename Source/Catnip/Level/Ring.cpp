// Fill out your copyright notice in the Description page of Project Settings.


#include "Ring.h"

#include "RingHandler.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ARing::ARing()
{
	//this->bVisible = true;
	this->RingResolution = 12;

	this->RotateSpeedMin = -25.0f;
	this->RotateSpeedMax = 25.0f;
	this->RotateSpeedRerollZone = 5.0f;

	this->LastOpacity = 1.0f;
	this->RequiredOpacity = 0.0f;

	this->bDebugDisableRotation = false;

	this->SceneComponent = UObject::CreateDefaultSubobject<USceneComponent>(TEXT("RingSceneComponent"));
	Super::RootComponent = this->SceneComponent;

	this->SplineComponent = UObject::CreateDefaultSubobject<USplineComponent>(TEXT("RingSplineComponent"));
	this->SplineComponent->SetMobility(EComponentMobility::Movable);
	this->SplineComponent->SetClosedLoop(true, false);
	this->SplineComponent->SetupAttachment(Super::RootComponent);

	Super::PrimaryActorTick.bCanEverTick = true;
}

void ARing::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	//this->UpdatePoints(nullptr, 500.0f);
#endif
}

void ARing::BeginPlay()
{
	Super::BeginPlay();

	this->StaticMeshComponents.Empty();

	//do
	//{
	//	this->RotateSpeed = FMath::RandRange(this->RotateSpeedMin, this->RotateSpeedMax);
	//} while (FMath::Abs(this->RotateSpeed) < this->RotateSpeedRerollZone);
}

#if 0
void ARing::UpdateColor(FLinearColor Color)
{
	if (this->MaterialInstanceDynamic == nullptr)
	{
		return;
	}
	this->MaterialInstanceDynamic->SetVectorParameterValue(TEXT("Color"), Color);
}
#endif

void ARing::InitRing(FRingSpawnState *State)
{
	if (State == nullptr || State->Mesh == nullptr || this->SplineComponent == nullptr || this->RingResolution <= 0.0f)
	{
		ensure(false);
		return;
	}
	if (this->SplineComponent->GetNumberOfSplinePoints() > 0)
	{
		this->SplineComponent->ClearSplinePoints();
	}

	FVector ActorLocation = Super::GetActorLocation();
	FRotator ActorRotation = Super::GetActorRotation();
	auto CreateStaticMesh = [&](FVector Location, FVector Scale)
	{
		UStaticMeshComponent *StaticMeshComponent = NewObject<UStaticMeshComponent>(this->SplineComponent);
		StaticMeshComponent->SetCastShadow(false);
		StaticMeshComponent->SetStaticMesh(State->Mesh);
		StaticMeshComponent->SetMobility(EComponentMobility::Movable);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComponent->SetWorldScale3D(Scale);
		StaticMeshComponent->SetWorldLocationAndRotation(Location, FRotator::ZeroRotator);
		StaticMeshComponent->AttachToComponent(this->SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
		if (this->MaterialInstanceDynamic == nullptr && State->MaterialInterface != nullptr)
		{
			this->MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(State->MaterialInterface, this);
			this->MaterialInstanceDynamic->SetVectorParameterValue(TEXT("Color"), State->Color);
		}
		if (this->MaterialInstanceDynamic != nullptr)
		{
			StaticMeshComponent->SetMaterial(0, this->MaterialInstanceDynamic);
		}
		StaticMeshComponent->RegisterComponent();
		return StaticMeshComponent;
	};

	// Get rotation offset.
	float RotationOffset;
	switch (State->OffsetType)
	{
	case ERingOffsetType::Fixed:
		RotationOffset = State->RotationOffset;
		break;
	case ERingOffsetType::Random:
		RotationOffset = FMath::RandRange(-State->RotationOffset, State->RotationOffset);
		break;
	case ERingOffsetType::Incremental:
		RotationOffset = State->RotationOffset * State->OffsetCounter;
		++State->OffsetCounter;
		break;
	default:
		RotationOffset = 0.0f;
		break;
	}

	// Get rotation speed.
	do
	{
		this->RotateSpeed = FMath::RandRange(State->RotationSpeedMin, State->RotationSpeedMax);
		if (State->RotationForceRerollMin < (State->RotationSpeedMax - State->RotationSpeedMin))
		{
			ensure(false);
			break;
		}
	} while (FMath::Abs(this->RotateSpeed) <= State->RotationForceRerollMin);

	// Spawn mesh.
	if (State->MeshType == ERingMeshType::SingleMesh)
	{
		UStaticMeshComponent *StaticMeshComponent = CreateStaticMesh(ActorLocation, FVector(State->Radius) * 0.2f);
		StaticMeshComponent->AddLocalRotation(FRotator(0.0f, 0.0f, RotationOffset));
		this->StaticMeshComponents.Add(StaticMeshComponent);
	}
	else if(State->MeshType == ERingMeshType::MultipleMesh)
	{
		constexpr float PI2 = PI * 2.0f;
		const float Increment = PI2 / this->RingResolution;
		for (int i = 0; i < this->RingResolution; ++i)
		{
			float Sin, Cos;
			FMath::SinCos(&Sin, &Cos, Increment * i + RotationOffset);

			FVector Point = ActorRotation.RotateVector(FVector(0.0f, Sin, Cos)) * State->Radius + ActorLocation;
			this->SplineComponent->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);
			this->StaticMeshComponents.Add(CreateStaticMesh(Point, FVector::OneVector));
		}
		this->SplineComponent->UpdateSpline();
	}
}

#if 0
void ARing::UpdatePoints(UStaticMesh *Mesh, bool bSingleMesh, float Radius)
{
	if (Mesh == nullptr)
	{
		return;
	}
	if (!ensure(this->SplineComponent != nullptr))
	{
		return;
	}
	if (!ensure(this->RingResolution > 0.0f))
	{
		return;
	}

	if (this->SplineComponent->GetNumberOfSplinePoints() > 0)
	{
		this->SplineComponent->ClearSplinePoints();
	}
	FVector ActorLocation = Super::GetActorLocation();
	FRotator ActorRotation = Super::GetActorRotation();

	auto CreateStaticMesh = [&](FVector Location, FVector Scale)
	{
		UStaticMeshComponent *StaticMeshComponent = NewObject<UStaticMeshComponent>(this->SplineComponent);
		StaticMeshComponent->SetCastShadow(false);
		StaticMeshComponent->SetStaticMesh(Mesh);
		StaticMeshComponent->SetMobility(EComponentMobility::Movable);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComponent->SetWorldScale3D(Scale);
		StaticMeshComponent->SetWorldLocationAndRotation(Location, FRotator::ZeroRotator);
		StaticMeshComponent->AttachToComponent(this->SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
		if (this->MaterialInstanceDynamic == nullptr && State->MaterialInterface != nullptr)
		{
			this->MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(State->MaterialInterface, this);
		}
		if (this->MaterialInstanceDynamic != nullptr)
		{
			StaticMeshComponent->SetMaterial(0, this->MaterialInstanceDynamic);
		}
		StaticMeshComponent->RegisterComponent();
		return StaticMeshComponent;
	};

	if (bSingleMesh)
	{
		this->StaticMeshComponents.Add(CreateStaticMesh(ActorLocation, FVector(Radius) * 0.15f));
	}
	else
	{
		constexpr float PI2 = PI * 2.0f;
		const float Increment = PI2 / this->RingResolution;
		const float AngleOffset = FMath::RandRange(0.0f, PI2);
		for (int i = 0; i < this->RingResolution; ++i)
		{
			float Sin, Cos;
			FMath::SinCos(&Sin, &Cos, Increment * i + AngleOffset);

			FVector Point = ActorRotation.RotateVector(FVector(0.0f, Sin, Cos)) * Radius + ActorLocation;
			this->SplineComponent->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);
			this->StaticMeshComponents.Add(CreateStaticMesh(Point, FVector::OneVector));
		}
		this->SplineComponent->UpdateSpline();
	}
}
#endif

void ARing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Rotate the ring.
	if ((!WITH_EDITOR || !this->bDebugDisableRotation) && !FMath::IsNearlyZero(this->RotateSpeed))
	{
		Super::AddActorLocalRotation(FRotator(0.0f, 0.0f, this->RotateSpeed * DeltaTime));
	}
	if (this->MaterialInstanceDynamic == nullptr || FMath::IsNearlyEqual(this->LastOpacity, this->RequiredOpacity))
	{
		return;
	}

	//float DistanceSquared = (this->PawnLocation - Super::GetActorLocation()).SizeSquared();
	//float Percentage = 1.0f - FMath::Clamp(DistanceSquared / FMath::Square(this->RingFadeDistance), 0.0f, 1.0f);
	//this->MaterialInstanceDynamic->SetScalarParameterValue(TEXT("OpacityPercentage"), Percentage);

	this->MaterialInstanceDynamic->SetScalarParameterValue(TEXT("OpacityPercentage"), this->RequiredOpacity);
	this->LastOpacity = this->RequiredOpacity;

	//if (this->bVisible == FMath::IsNearlyZero(this->LastOpacity))
	//{
	//	this->bVisible = !this->bVisible;
	//	for (USceneComponent *Component : this->SplineComponent->GetAttachChildren())
	//	{
	//		Component->SetVisibility(this->bVisible);
	//	}
	//}
}

