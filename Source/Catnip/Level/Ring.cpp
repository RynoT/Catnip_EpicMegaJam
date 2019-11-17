// Fill out your copyright notice in the Description page of Project Settings.


#include "Ring.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ARing::ARing()
{
	this->RingRadius = 500.0f;
	this->RingResolution = 12;

	this->LastOpacity = 1.0f;
	this->RequiredOpacity = 0.0f;

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
	this->UpdatePoints(nullptr);
#endif
}

void ARing::UpdatePoints(UStaticMesh* Mesh)
{
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
	FVector Location = Super::GetActorLocation();
	FRotator Rotation = Super::GetActorRotation();

	constexpr float PI2 = PI * 2.0f;
	const float Increment = PI2 / this->RingResolution;
	const float AngleOffset = FMath::RandRange(0.0f, PI2);
	for (int i = 0; i < this->RingResolution; ++i)
	{
		float Sin, Cos;
		FMath::SinCos(&Sin, &Cos, Increment * i + AngleOffset);

		FVector Point = Rotation.RotateVector(FVector(0.0f, Sin, Cos)) * this->RingRadius + Location;
		this->SplineComponent->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);

		if (Mesh == nullptr)
		{
			continue;
		}
		this->StaticMeshComponent = NewObject<UStaticMeshComponent>(this->SplineComponent);
		this->StaticMeshComponent->SetCastShadow(false);
		this->StaticMeshComponent->SetStaticMesh(Mesh);
		this->StaticMeshComponent->SetMobility(EComponentMobility::Movable);
		this->StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		this->StaticMeshComponent->SetWorldLocationAndRotation(Point, FRotator::ZeroRotator);
		this->StaticMeshComponent->AttachToComponent(this->SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
		if (this->MaterialInstanceDynamic == nullptr && this->MaterialInterface != nullptr)
		{
			this->MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(this->MaterialInterface, this);
		}
		if (this->MaterialInstanceDynamic != nullptr)
		{
			this->StaticMeshComponent->SetMaterial(0, this->MaterialInstanceDynamic);
		}
		this->StaticMeshComponent->RegisterComponent();
	}
	this->SplineComponent->UpdateSpline();
}

void ARing::BeginPlay()
{
	Super::BeginPlay();
}

void ARing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->MaterialInstanceDynamic == nullptr || FMath::IsNearlyEqual(this->LastOpacity, this->RequiredOpacity))
	{
		return;
	}

	//float DistanceSquared = (this->PawnLocation - Super::GetActorLocation()).SizeSquared();
	//float Percentage = 1.0f - FMath::Clamp(DistanceSquared / FMath::Square(this->RingFadeDistance), 0.0f, 1.0f);
	//this->MaterialInstanceDynamic->SetScalarParameterValue(TEXT("OpacityPercentage"), Percentage);

	this->MaterialInstanceDynamic->SetScalarParameterValue(TEXT("OpacityPercentage"), this->RequiredOpacity);
	this->LastOpacity = this->RequiredOpacity;
}

