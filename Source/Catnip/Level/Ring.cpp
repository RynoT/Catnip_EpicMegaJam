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
	this->UpdatePoints(nullptr, 500.0f);
#endif
}

void ARing::BeginPlay()
{
	Super::BeginPlay();

	do
	{
		this->RotateSpeed = FMath::RandRange(this->RotateSpeedMin, this->RotateSpeedMax);
	} while (FMath::Abs(this->RotateSpeed) < this->RotateSpeedRerollZone);
}

void ARing::UpdateColor(FLinearColor Color)
{
	if (this->MaterialInstanceDynamic == nullptr)
	{
		return;
	}
	this->MaterialInstanceDynamic->SetVectorParameterValue(TEXT("Color"), Color);
}

void ARing::UpdatePoints(UStaticMesh *Mesh, float Radius)
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

		FVector Point = Rotation.RotateVector(FVector(0.0f, Sin, Cos)) * Radius + Location;
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

void ARing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Rotate the ring.
	if (!WITH_EDITOR || !this->bDebugDisableRotation)
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

